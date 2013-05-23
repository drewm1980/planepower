import rawe
from rawe.ocp.Ocp import Mhe
import casadi as C
import carouselModel

from common_conf import Ts

mheHorizonN = 10

mheIntOpts = rawe.RtIntegratorOptions()
mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_GL2'
#mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_RIIA3'
mheIntOpts['NUM_INTEGRATOR_STEPS'] = 4
mheIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 3
mheIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS_INIT'] = 0
mheIntOpts['LINEAR_ALGEBRA_SOLVER'] = 'HOUSEHOLDER_QR'
mheIntOpts['UNROLL_LINEAR_SOLVER'] = False
mheIntOpts['IMPLICIT_INTEGRATOR_MODE'] = 'IFTR'

mheOpts = rawe.OcpExportOptions()
mheOpts['HESSIAN_APPROXIMATION'] = 'GAUSS_NEWTON'
mheOpts['DISCRETIZATION_TYPE'] = 'MULTIPLE_SHOOTING'
mheOpts['QP_SOLVER'] = 'QP_QPOASES'
mheOpts['HOTSTART_QP'] = True
mheOpts['SPARSE_QP_SOLUTION'] = 'CONDENSING'
#mheOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING_U2'
#mheOpts['MAX_NUM_QP_ITERATIONS'] = '30'
mheOpts['FIX_INITIAL_STATE'] = False
#mheOpts['CG_USE_VARIABLE_WEIGHTING_MATRIX'] = False
#mheOpts['CG_USE_C99'] = True

# normal measurements
measNames  = ['marker_positions','IMU_angular_velocity','IMU_acceleration']
measNames += ['r','cos_delta','sin_delta','aileron','elevator']
measNames += ['daileron', 'delevator', 'dmotor_torque', 'dddr']

endMeasNames  = ['IMU_angular_velocity']
endMeasNames += ['r','cos_delta','sin_delta','aileron','elevator']

## full state feedback
#endMeasNames = ['x', 'y', 'z', 'e11', 'e12', 'e13', 'e21', 'e22', 'e23', 'e31', 'e32', 'e33', 'dx', 'dy', 'dz', 'w1', 'w2', 'w3', 'ddelta', 'r', 'dr', 'aileron', 'elevator', 'motor_torque', 'ddr', 'cos_delta', 'sin_delta']
#measNames = endMeasNames + ['daileron', 'delevator', 'dmotor_torque', 'dddr']


def makeMhe(dae):
    mhe = Mhe(dae, N=mheHorizonN, ts=Ts, measNames=measNames, endMeasNames=endMeasNames)

    mhe.constrain(mhe['ConstR1'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR2'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR3'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR4'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR5'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR6'],'==',0, when='AT_END')

    mhe.constrain(mhe['c'],'==',0, when='AT_END')
    mhe.constrain(mhe['cdot'],'==',0, when='AT_END')

    mhe.constrain(mhe['ConstDelta'],'==',0, when='AT_END')

#    cgOpts = {'CXX':'g++', 'CC':'gcc'}
    cgOpts = {'CXX':'clang++', 'CC':'clang'}
    mheRT = mhe.exportCode(codegenOptions=cgOpts,ocpOptions=mheOpts,integratorOptions=mheIntOpts)

#    RintMeas = rawe.RtIntegrator(dae,ts=Ts, options=intOpts, measurements=measurements)
#    RintMeasEnd = rawe.RtIntegrator(dae,ts=Ts, options=intOpts, measurements=measurementsEND)

    return mheRT
