import rawe
from rawe.ocp import Ocp
import casadi as C
import carouselModel

def makeMhe(dae,N,dt,nSteps,iType):
    mhe = Ocp(dae, N=N, ts=dt)

#    measurements = C.veccat( [dae[n] for n in ['marker positions','IMU acceleration','IMU angular velocity']])
#    measurements = C.veccat( [measurements,
#                              dae['cos_delta'],
#                              dae['aileron'],
#                              dae['elevator'],
#                              dae['motor_torque'],
#                              dae['daileron'],
#                              dae['delevator']])
#
#    mhe.minimizeLsq(measurements)
#    mhe.minimizeLsqEndTerm(measurements)
    
    xref = C.veccat( [mhe[n] for n in dae.xNames()])
    uref = C.veccat( [mhe[n] for n in dae.uNames()])
    
#    dae['measurements'] = C.veccat([xref,uref])
#    dae['measurementsN'] = xref
    
    mhe.minimizeLsq(C.veccat([xref,uref]))
    mhe.minimizeLsqEndTerm(xref)
    
    mhe.constrain(mhe['ConstR1'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR2'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR3'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR4'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR5'],'==',0, when='AT_END')
    mhe.constrain(mhe['ConstR6'],'==',0, when='AT_END')

    mhe.constrain(mhe['Const'],'==',0, when='AT_END')
    mhe.constrain(mhe['dConst'],'==',0, when='AT_END')

    mhe.constrain(mhe['ConstDelta'],'==',0, when='AT_END')
    
    intOpts = rawe.RtIntegratorOptions()
    intOpts['INTEGRATOR_TYPE'] = iType
    intOpts['NUM_INTEGRATOR_STEPS'] = nSteps*N
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
    ocpOpts['SPARSE_QP_SOLUTION'] = 'CONDENSING'
#   ocpOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING_U2'
#   ocpOpts['MAX_NUM_QP_ITERATIONS'] = '30'
    ocpOpts['FIX_INITIAL_STATE'] = False
#    ocpOpts['CG_USE_VARIABLE_WEIGHTING_MATRIX'] = False
#    ocpOpts['CG_USE_C99'] = True


#    cgOpts = {'CXX':'g++', 'CC':'gcc'}
    cgOpts = {'CXX':'clang++', 'CC':'clang'}
    mheRT = mhe.exportCode(codegenOptions=cgOpts,ocpOptions=ocpOpts,integratorOptions=intOpts)
    return mheRT


if __name__=='__main__':
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)
    dae = carouselModel.makeModel(dae,conf)
    OcpRt = makeMhe(dae,10,0.1)

