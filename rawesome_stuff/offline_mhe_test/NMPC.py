import rawe

import MHE

from rawe.models.arianne_conf import makeConf
from offline_mhe_test import carouselModel

print "Using the updated NMPC formulation"

mpcHorizonN = 25 + 15

# In this case it is the same as for the MHE, but it might be different!
samplingTime = MHE.samplingTime

#
# Options for model integration are the same as in the MHE
#
mpcIntOpts = MHE.mheIntOpts

#
# OCP export options, NMPC specific
#
mpcOpts = rawe.OcpExportOptions()
mpcOpts['HESSIAN_APPROXIMATION'] = 'GAUSS_NEWTON'
mpcOpts['DISCRETIZATION_TYPE'] = 'MULTIPLE_SHOOTING'
mpcOpts['QP_SOLVER'] = 'QP_QPOASES'
mpcOpts['HOTSTART_QP'] = True
mpcOpts['MAX_NUM_QP_ITERATIONS'] = 50
mpcOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING_N2'
mpcOpts['FIX_INITIAL_STATE'] = True

# Define the weights
mpcSigmas = {}
for name in ['x', 'y', 'z']: mpcSigmas[name] = 1e0 #1e-1
for name in ['dx', 'dy', 'dz']: mpcSigmas[name] = 1e0 #1.e2
for name in ['e11', 'e12', 'e13', 'e21', 'e22', 'e23', 'e31', 'e32', 'e33']: mpcSigmas[name] = 1.0
for name in ['w_bn_b_x', 'w_bn_b_y', 'w_bn_b_z']: mpcSigmas[name] = 1.0
mpcSigmas['r'] = 1.0
mpcSigmas['dr'] = 1.0
mpcSigmas['ddr'] = 1.0
mpcSigmas['cos_delta'] = mpcSigmas['sin_delta'] = 1.0
mpcSigmas['ddelta'] = 1.0
mpcSigmas['motor_torque'] = 1e0
mpcSigmas['aileron'] = mpcSigmas['elevator'] = 1e0 # 1e1

mpcSigmas['dmotor_torque'] = 1.0
mpcSigmas['dddr'] = 1.0
mpcSigmas['daileron'] = mpcSigmas['delevator'] = 1e0 #1e2

mpcWeightScaling = 1e-3

mpcWeights = {}
for name in mpcSigmas:
    mpcWeights[name] = mpcWeightScaling / mpcSigmas[name] ** 2

def makeNmpc(Ts = None, propertiesDir = '../../properties'):
    conf = makeConf()
    conf['stabilize_invariants'] = False
    dae = carouselModel.makeModel(conf, propertiesDir = propertiesDir)
    
    if Ts is None:
        execSamplingTime = samplingTime
    else:
        execSamplingTime = Ts
    
    mpc = rawe.Mpc(dae, N = mpcHorizonN, ts = execSamplingTime)

    # Actuator limitations
    aileron_bound = conf['aileron_bound']
    elevator_bound = conf['elevator_bound']
    
    mpc_aileron_bound = aileron_bound / 2
    mpc_elevator_bound = elevator_bound / 2

    mpc.constrain(-mpc_aileron_bound, '<=', mpc['aileron'], '<=', mpc_aileron_bound)
    mpc.constrain(-mpc_elevator_bound, '<=', mpc['elevator'], '<=', mpc_elevator_bound)
    
    mpc_daileron_bound = aileron_bound * 2
    mpc_delevator_bound = elevator_bound * 2

    mpc.constrain(-mpc_daileron_bound, '<=', mpc['daileron'], '<=', mpc_daileron_bound)
    mpc.constrain(-mpc_delevator_bound, '<=', mpc['delevator'], '<=', mpc_delevator_bound)

    # We cannot influence the rotational speed of the carousel, nor torque,
    # thus whatever the estimator says, we don't want to influence that
    mpc.constrain(mpc['dmotor_torque'], '==', 0)
    mpc.constrain(mpc['dr'], '==', 0, when = 'AT_START')
    mpc.constrain(mpc['ddr'], '==', 0, when = 'AT_START')
    mpc.constrain(mpc['dddr'], '==', 0)

    return mpc

def makeNmpcRT(Ts, lqrDae = None, propertiesDir = '../../properties', cgOptions = None):
    if cgOptions is None:
        cgOptions = {'CXX': 'clang++', 'CC': 'clang',
                     'CXXFLAGS': '-O3 -fPIC -finline-functions',
                     'CFLAGS': '-O3 -fPIC -finline-functions'}
    mpc = makeNmpc(Ts = Ts, propertiesDir = propertiesDir)
    return rawe.MpcRT(mpc, lqrDae = lqrDae, ocpOptions = mpcOpts,
                      integratorOptions = mpcIntOpts, codegenOptions = cgOptions)
