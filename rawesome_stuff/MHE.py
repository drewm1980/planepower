import rawe
from rawe.ocp import Ocp
import casadi as C
import carouselModel

if __name__=='__main__':
    dae = carouselModel.makeModel()

    mhe = Ocp(dae, N=10, ts=0.1)

    measurements = C.veccat( [dae[n] for n in ['marker positions','IMU acceleration','IMU angular velocity']])
    measurements = C.veccat( [measurements,
                              dae['cos_delta'],
                              dae['aileron'],
                              dae['elevator'],
                              dae['motor_torque'],
                              dae['daileron'],
                              dae['delevator']])

    mhe.minimizeLsq(measurements)
    mhe.minimizeLsqEndTerm(measurements)
    e11 = dae['e11']
    e12 = dae['e12']
    e13 = dae['e13']

    e21 = dae['e21']
    e22 = dae['e22']
    e23 = dae['e23']

    e31 = dae['e31']
    e32 = dae['e32']
    e33 = dae['e33']

    x = dae['x']
    y = dae['y']
    z = dae['z']

    dx = dae['dx']
    dy = dae['dy']
    dz = dae['dz']

    r = dae['r']

    cos_delta = dae['cos_delta']
    sin_delta = dae['sin_delta']

    ConstR1 = e11*e11 + e12*e12 + e13*e13 - 1
    ConstR2 = e11*e21 + e12*e22 + e13*e23
    ConstR3 = e11*e31 + e12*e32 + e13*e33
    ConstR4 = e21*e21 + e22*e22 + e23*e23 - 1
    ConstR5 = e21*e31 + e22*e32 + e23*e33
    ConstR6 = e31*e31 + e32*e32 + e33*e33 - 1

    Const = - r*r/2 + x*x/2 + y*y/2 + z*z/2
    dConst = dx*x + dy*y + dz*z

    ConstDelta = cos_delta**2+sin_delta**2 - 1

    mhe.constrain(ConstR1,'==',0, when='AT_END')
    mhe.constrain(ConstR2,'==',0, when='AT_END')
    mhe.constrain(ConstR3,'==',0, when='AT_END')
    mhe.constrain(ConstR4,'==',0, when='AT_END')
    mhe.constrain(ConstR5,'==',0, when='AT_END')
    mhe.constrain(ConstR6,'==',0, when='AT_END')

    mhe.constrain(Const,'==',0, when='AT_END')
    mhe.constrain(dConst,'==',0, when='AT_END')

    mhe.constrain(ConstDelta,'==',0, when='AT_END')

    acadoOpts = [('HESSIAN_APPROXIMATION','GAUSS_NEWTON'),
                 ('DISCRETIZATION_TYPE','MULTIPLE_SHOOTING'),
                 ('HESSIAN_APPROXIMATION','GAUSS_NEWTON'),
                 ('DISCRETIZATION_TYPE','MULTIPLE_SHOOTING'),
                 ('INTEGRATOR_TYPE','INT_IRK_GL2'),
                 ('NUM_INTEGRATOR_STEPS','30'),
                 ('IMPLICIT_INTEGRATOR_NUM_ITS','3'),
                 ('IMPLICIT_INTEGRATOR_NUM_ITS_INIT','0'),
                 ('LINEAR_ALGEBRA_SOLVER','HOUSEHOLDER_QR'),
                 ('UNROLL_LINEAR_SOLVER','NO'),
                 ('IMPLICIT_INTEGRATOR_MODE','IFTR'),
                 ('SPARSE_QP_SOLUTION','CONDENSING'),
                 ('QP_SOLVER','QP_QPOASES'),
                 ('HOTSTART_QP','YES'),
                 ('GENERATE_TEST_FILE','YES'),
                 ('CG_USE_C99','YES'),
                 ('PRINTLEVEL','HIGH'),
                 ('CG_USE_VARIABLE_WEIGHTING_MATRIX','YES')]



    cgOpts = {'CXX':'g++', 'CC':'gcc'}
    mheRt = mhe.exportCode(cgOptions=cgOpts,acadoOptions=acadoOpts)
