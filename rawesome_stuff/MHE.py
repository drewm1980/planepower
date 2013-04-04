import rawe
import casadi as C
import carouselModel

if __name__=='__main__':
    dae = carouselModel.makeModel()

    from rawe.ocp import Ocp
    mhe = Ocp(dae, N=10, ts=0.1)
    mhe.constrain( mpc['motor_torque'], '==', 0 );
    mhe.constrain( mpc['ddr'], '==', 0 );
    mhe.constrain( -32767/1.25e6, '<=', mpc['aileron'] );
    mhe.constrain( mpc['aileron'], '<=', 32767/1.25e6 );
    mhe.constrain( -32767/2e5, '<=', mpc['elevator'] );
    mhe.constrain( mpc['elevator'], '<=', 32767/2e5 );
    mhe.constrain( -0.2, '<=', mpc['daileron'] );
    mhe.constrain( mpc['daileron'], '<=', 0.2 );
    mhe.constrain( -1, '<=', mpc['delevator'] );
    mhe.constrain( mpc['delevator'], '<=', 1 );

    measurements = C.veccat( [dae[n] for n in ['marker positions','IMU acceleration','IMU angular velocity','delta']])

    mhe.minimizeLsq(measurements)
    mhe.minimizeLsqEndTerm(measurements)

    #mhe.exportCode(CXX='clang++')
