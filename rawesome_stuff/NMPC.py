import rawe
import casadi as C

if __name__=='__main__':
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)

    from rawe.ocp import Ocp
    mpc = Ocp(dae, N=10, ts=0.1)
    mpc.constrain( mpc['motor_torque'], '==', 0 );
    mpc.constrain( mpc['ddr'], '==', 0 );
    mpc.constrain( -32767/1.25e6, '<=', mpc['aileron'] );
    mpc.constrain( mpc['aileron'], '<=', 32767/1.25e6 );
    mpc.constrain( -32767/2e5, '<=', mpc['elevator'] );
    mpc.constrain( mpc['elevator'], '<=', 32767/2e5 );
    mpc.constrain( -0.2, '<=', mpc['daileron'] );
    mpc.constrain( mpc['daileron'], '<=', 0.2 );
    mpc.constrain( -1, '<=', mpc['delevator'] );
    mpc.constrain( mpc['delevator'], '<=', 1 );

    ref = C.veccat( [dae[n] for n in ['x','y','z','dx','dy','dz','e11', 'e12', 'e13','e21', 'e22', 'e23','e31', 'e32', 'e33','w1','w2','w3']])

    myoptions=[('HESSIAN_APPROXIMATION','GAUSS_NEWTON'),
               ('DISCRETIZATION_TYPE','MULTIPLE_SHOOTING'),
               ('QP_SOLVER','QP_QPOASES'),
               ('HOTSTART_QP','NO'),
               ('INTEGRATOR_TYPE','INT_IRK_GL2'),
               ('NUM_INTEGRATOR_STEPS','40'),
               ('IMPLICIT_INTEGRATOR_NUM_ITS','3'),
               ('IMPLICIT_INTEGRATOR_NUM_ITS_INIT','0'),
               ('LINEAR_ALGEBRA_SOLVER','HOUSEHOLDER_QR'),
               ('UNROLL_LINEAR_SOLVER','NO'),
               ('IMPLICIT_INTEGRATOR_MODE','IFTR'),
               ('SPARSE_QP_SOLUTION','CONDENSING'),
               ('AX_NUM_QP_ITERATIONS','30'),
               ('GENERATE_TEST_FILE','YES'),
               ('GENERATE_SIMULINK_INTERFACE','YES'),
               ('GENERATE_MAKE_FILE','NO'),
               ('CG_USE_C99','YES')]

    mpc.minimizeLsq(ref)
    mpc.minimizeLsqEndTerm(ref)

    #mpc.exportCode(CXX='clang++')
