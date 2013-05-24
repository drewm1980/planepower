import rawe
import casadi as C
from rawe.ocp.Ocp import Mpc

import MHE
from common_conf import Ts

mpcHorizonN = MHE.mheHorizonN

mpcIntOpts = rawe.RtIntegratorOptions()
mpcIntOpts['INTEGRATOR_TYPE'] = MHE.mheIntOpts['INTEGRATOR_TYPE']
mpcIntOpts['NUM_INTEGRATOR_STEPS'] = MHE.mheIntOpts['NUM_INTEGRATOR_STEPS']
mpcIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 3
mpcIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS_INIT'] = 0
mpcIntOpts['LINEAR_ALGEBRA_SOLVER'] = 'HOUSEHOLDER_QR'
mpcIntOpts['UNROLL_LINEAR_SOLVER'] = False
mpcIntOpts['IMPLICIT_INTEGRATOR_MODE'] = 'IFTR'

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

def makeNmpc(dae,lqrDae):
    mpc = Mpc(dae, N=mpcHorizonN, ts=Ts, lqrDae=lqrDae)

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


#    xref = C.veccat( [mpc[n] for n in dae.xNames()])
#    uref = C.veccat( [mpc[n] for n in dae.uNames()])
#    mpc.minimizeLsq(C.veccat([xref,uref]))
#    mpc.minimizeLsqEndTerm(xref)

    cgOpts = {'CXX':'g++', 'CC':'gcc'}
    mpcRT = mpc.exportCode(codegenOptions=cgOpts,integratorOptions=mpcIntOpts,ocpOptions=mpcOpts)
    return mpcRT
