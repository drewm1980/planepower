import casadi as C
import rawe

import MHE
from common_conf import Ts
from highwind_carousel_conf import getConf
import carouselModel

mpcHorizonN = MHE.mheHorizonN

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

def makeNmpc(propertiesDir='../properties'):
    conf = getConf()
    conf['stabilize_invariants'] = False
    dae = carouselModel.makeModel(conf,propertiesDir=propertiesDir)
    mpc = rawe.Mpc(dae, N=mpcHorizonN, ts=Ts)

    mpc.constrain( mpc['ddr'], '==', 0 );
    mpc.constrain( -32767/1.25e6, '<=', mpc['aileron'] );
    mpc.constrain( mpc['aileron'], '<=', 32767/1.25e6 );
    mpc.constrain( -32767/2e5, '<=', mpc['elevator'] );
    mpc.constrain( mpc['elevator'], '<=', 32767/2e5 );
    mpc.constrain( -0.2, '<=', mpc['daileron'] );
    mpc.constrain( mpc['daileron'], '<=', 0.2 );
    mpc.constrain( -1, '<=', mpc['delevator'] );
    mpc.constrain( mpc['delevator'], '<=', 1 );
    mpc.constrain( 0, '<=', mpc['motor_torque'] );
    mpc.constrain( mpc['motor_torque'], '<=', 2000 );

    return mpc

def makeNmpcRT(lqrDae, propertiesDir='../properties', cgOptions = None):
    if cgOptions is None:
        cgOptions= {'CXX':'clang++', 'CC':'clang',
                    'CXXFLAGS':'-O3 -fPIC -finline-functions',
                    'CFLAGS':'-O3 -fPIC -finline-functions'}
    mpc = makeNmpc(propertiesDir=propertiesDir)
    return rawe.MpcRT(mpc, lqrDae, ocpOptions=mpcOpts,
                      integratorOptions=mpcIntOpts, codegenOptions=cgOptions)
