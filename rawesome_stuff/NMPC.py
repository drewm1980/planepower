import rawe
import casadi as C

def makeNmpc(dae,N,dt):
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
    mpc.constrain( mpc['motor_torque'], '<=', 20 );

    acadoOpts=[('HESSIAN_APPROXIMATION','GAUSS_NEWTON'),
               ('DISCRETIZATION_TYPE','MULTIPLE_SHOOTING'),
               ('QP_SOLVER','QP_QPOASES'),
               ('HOTSTART_QP','NO'),
               ('INTEGRATOR_TYPE','INT_IRK_GL2'),
               ('NUM_INTEGRATOR_STEPS',str(40*N)),
               ('IMPLICIT_INTEGRATOR_NUM_ITS','3'),
               ('IMPLICIT_INTEGRATOR_NUM_ITS_INIT','0'),
               ('LINEAR_ALGEBRA_SOLVER','HOUSEHOLDER_QR'),
               ('UNROLL_LINEAR_SOLVER','NO'),
               ('IMPLICIT_INTEGRATOR_MODE','IFTR'),
               ('SPARSE_QP_SOLUTION','CONDENSING'),
#               ('SPARSE_QP_SOLUTION','FULL_CONDENSING_U2'),
#               ('AX_NUM_QP_ITERATIONS','30'),
               ('FIX_INITIAL_STATE','YES'),
               ('GENERATE_TEST_FILE','YES'),
               ('GENERATE_SIMULINK_INTERFACE','YES'),
               ('GENERATE_MAKE_FILE','NO'),
               ('CG_USE_C99','YES')]
    

    xref = C.veccat( [mpc[n] for n in dae.xNames()])
    uref = C.veccat( [mpc[n] for n in dae.uNames()])
    mpc.minimizeLsq(C.veccat([xref,uref]))
    mpc.minimizeLsqEndTerm(xref)

    cgOpts = {'CXX':'g++', 'CC':'gcc'}
    mpcRT = mpc.exportCode(codegenOptions=cgOpts,acadoOptions=acadoOpts)
    return mpcRT

if __name__=='__main__':
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)

    OcpRt = makeNmpc(dae,10,0.1)
