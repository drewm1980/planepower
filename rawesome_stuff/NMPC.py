import rawe
import casadi as C

def makeNmpc(dae,N,dt,nSteps,iType):
    from rawe.ocp import Ocp
    mpc = Ocp(dae, N=N, ts=dt)

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

    intOpts = rawe.RtIntegratorOptions()
    intOpts['INTEGRATOR_TYPE'] = iType
    intOpts['NUM_INTEGRATOR_STEPS'] = nSteps
    intOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 3
    intOpts['IMPLICIT_INTEGRATOR_NUM_ITS_INIT'] = 0
    intOpts['LINEAR_ALGEBRA_SOLVER'] = 'HOUSEHOLDER_QR'
    intOpts['UNROLL_LINEAR_SOLVER'] = False
    intOpts['IMPLICIT_INTEGRATOR_MODE'] = 'IFTR'

    ocpOpts = rawe.OcpExportOptions()
    ocpOpts['HESSIAN_APPROXIMATION'] = 'GAUSS_NEWTON'
    ocpOpts['DISCRETIZATION_TYPE'] = 'MULTIPLE_SHOOTING'
    ocpOpts['QP_SOLVER'] = 'QP_QPOASES'
    ocpOpts['HOTSTART_QP'] = True
    ocpOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING'
#   ocpOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING_U2'
#   ocpOpts['MAX_NUM_QP_ITERATIONS'] = '30'
    ocpOpts['FIX_INITIAL_STATE'] = True
#    ocpOpts['CG_USE_C99'] = True

    xref = C.veccat( [mpc[n] for n in dae.xNames()])
    uref = C.veccat( [mpc[n] for n in dae.uNames()])
    mpc.minimizeLsq(C.veccat([xref,uref]))
    mpc.minimizeLsqEndTerm(xref)

    cgOpts = {'CXX':'g++', 'CC':'gcc'}
    mpcRT = mpc.exportCode(codegenOptions=cgOpts,integratorOptions=intOpts,ocpOptions=ocpOpts)
    return mpcRT, intOpts

if __name__=='__main__':
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)

    mpcRt = makeNmpc(dae,10,0.1)

