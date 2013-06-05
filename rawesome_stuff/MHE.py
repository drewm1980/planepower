import casadi as C
import rawe

import carouselModel
from highwind_carousel_conf import getConf
from common_conf import Ts

mheHorizonN = 20

mheIntOpts = rawe.RtIntegratorOptions()
mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_GL2'
#mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_RIIA3'
mheIntOpts['NUM_INTEGRATOR_STEPS'] = 20
#mheIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 3
#mheIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS_INIT'] = 0
#mheIntOpts['LINEAR_ALGEBRA_SOLVER'] = 'HOUSEHOLDER_QR'
#mheIntOpts['UNROLL_LINEAR_SOLVER'] = False
#mheIntOpts['IMPLICIT_INTEGRATOR_MODE'] = 'IFTR'

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

# measurements
measX = ['marker_positions','cos_delta','sin_delta','IMU_angular_velocity','IMU_acceleration']
measX += ['aileron','elevator']

measU = ['daileron', 'delevator', 'dmotor_torque', 'dddr']


def makeMhe(propertiesDir='../properties'):
    conf = getConf()
    conf['stabilize_invariants'] = False
    dae = carouselModel.makeModel(conf,propertiesDir=propertiesDir)

    mhe = rawe.Mhe(dae, N=mheHorizonN, ts=Ts, yxNames=measX, yuNames=measU)

    mhe.constrain(mhe['ConstR1'],'==',0, when='AT_START')
    mhe.constrain(mhe['ConstR2'],'==',0, when='AT_START')
    mhe.constrain(mhe['ConstR3'],'==',0, when='AT_START')
    mhe.constrain(mhe['ConstR4'],'==',0, when='AT_START')
    mhe.constrain(mhe['ConstR5'],'==',0, when='AT_START')
    mhe.constrain(mhe['ConstR6'],'==',0, when='AT_START')

    mhe.constrain(mhe['c'],'==',0, when='AT_START')
    mhe.constrain(mhe['cdot'],'==',0, when='AT_START')

    mhe.constrain(mhe['ConstDelta'],'==',0, when='AT_START')

    return mhe

def makeMheRT(propertiesDir='../properties', cgOptions = None):
    if cgOptions is None:
        cgOptions= {'CXX':'clang++', 'CC':'clang',
                    'CXXFLAGS':'-O3 -fPIC -finline-functions',
                    'CFLAGS':'-O3 -fPIC -finline-functions'}
    mhe = makeMhe(propertiesDir=propertiesDir)
    return rawe.MheRT(mhe, ocpOptions=mheOpts, integratorOptions=mheIntOpts, codegenOptions=cgOptions)
