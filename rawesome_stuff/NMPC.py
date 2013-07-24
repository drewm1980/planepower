import numpy
import casadi as C
import rawe

import MHE
from common_conf import Ts
from rawe.models.arianne_conf import makeConf
import carouselModel

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
