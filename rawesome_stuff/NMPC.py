import numpy
import casadi as C
import rawe

import MHE
from common_conf import Ts
from rawe.models.arianne_conf import makeConf
from offline_mhe_test import carouselModel

mpcHorizonN = 15

mpcIntOpts = rawe.RtIntegratorOptions()
mpcIntOpts['INTEGRATOR_TYPE'] = MHE.mheIntOpts['INTEGRATOR_TYPE']
mpcIntOpts['NUM_INTEGRATOR_STEPS'] = MHE.mheIntOpts['NUM_INTEGRATOR_STEPS']
#mpcIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 3
#mpcIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS_INIT'] = 0
#mpcIntOpts['LINEAR_ALGEBRA_SOLVER'] = 'HOUSEHOLDER_QR'
#mpcIntOpts['UNROLL_LINEAR_SOLVER'] = False
#mpcIntOpts['IMPLICIT_INTEGRATOR_MODE'] = 'IFTR'

mpcOpts = rawe.OcpExportOptions()
mpcOpts['HESSIAN_APPROXIMATION'] = 'GAUSS_NEWTON'
mpcOpts['DISCRETIZATION_TYPE'] = 'MULTIPLE_SHOOTING'
mpcOpts['QP_SOLVER'] = 'QP_QPOASES'
mpcOpts['HOTSTART_QP'] = True
mpcOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING'
#mpcOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING_U2'
#mpcOpts['MAX_NUM_QP_ITERATIONS'] = '30'
mpcOpts['FIX_INITIAL_STATE'] = True
#mpcOpts['CG_USE_C99'] = True

# Define the weights
mpcSigmas = {}
for name in ['x','y','z']: mpcSigmas[name] = 1.0
for name in ['dx','dy','dz']: mpcSigmas[name] = 1.0
for name in ['e11', 'e12', 'e13', 'e21', 'e22', 'e23', 'e31', 'e32', 'e33']: mpcSigmas[name] = 1.0
for name in ['w_bn_b_x','w_bn_b_y','w_bn_b_z']: mpcSigmas[name] = 1.0
mpcSigmas['r'] = 1.0
mpcSigmas['dr'] = 1.0
mpcSigmas['ddr'] = 1.0
mpcSigmas['cos_delta'] = mpcSigmas['sin_delta'] = 1.0
mpcSigmas['ddelta'] = 1.0
mpcSigmas['motor_torque'] = 1e1
mpcSigmas['aileron'] = mpcSigmas['elevator'] = 1e-2

mpcSigmas['dmotor_torque'] = 1.0
mpcSigmas['dddr'] = 1.0
mpcSigmas['daileron'] = mpcSigmas['delevator'] = 1.0

mpcWeights = {}
for name in mpcSigmas:
    mpcWeights[name] = 1.0/mpcSigmas[name]**2
for name in [ 'x', 'y', 'z']: mpcWeights[name] *= 1e-1
for name in ['dx','dy','dz']: mpcWeights[name] *= 1e1

def radians(x):
    return float(numpy.radians(x))

def makeNmpc(propertiesDir='../properties'):
    conf = makeConf()
    conf['stabilize_invariants'] = False
    dae = carouselModel.makeModel(conf,propertiesDir=propertiesDir)
    mpc = rawe.Mpc(dae, N=mpcHorizonN, ts=Ts)

#    mpc.constrain( mpc['dddr'], '==', 0 );
    mpc.constrain( radians(-5.0), '<=', mpc['aileron'],  '<=', radians(5.0) );
    mpc.constrain( radians(-5.0), '<=', mpc['elevator'], '<=', radians(5.0) );
    mpc.constrain( -0.2*100, '<=', mpc['daileron'], '<=', 0.2*100 );
    mpc.constrain( -1*100, '<=', mpc['delevator'], '<=', 1*100 );
    mpc.constrain( 0, '<=', mpc['motor_torque'], '<=', 2000 );

    return mpc

def makeNmpcRT(lqrDae, propertiesDir='../properties', cgOptions = None):
    if cgOptions is None:
        cgOptions= {'CXX':'clang++', 'CC':'clang',
                    'CXXFLAGS':'-O3 -fPIC -finline-functions',
                    'CFLAGS':'-O3 -fPIC -finline-functions'}
    mpc = makeNmpc(propertiesDir=propertiesDir)
    return rawe.MpcRT(mpc, lqrDae, ocpOptions=mpcOpts,
                      integratorOptions=mpcIntOpts, codegenOptions=cgOptions)
