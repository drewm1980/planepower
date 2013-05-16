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
    
#    mhe.constrain(mhe['ConstR1'],'==',0, when='AT_END')
#    mhe.constrain(mhe['ConstR2'],'==',0, when='AT_END')
#    mhe.constrain(mhe['ConstR3'],'==',0, when='AT_END')
#    mhe.constrain(mhe['ConstR4'],'==',0, when='AT_END')
#    mhe.constrain(mhe['ConstR5'],'==',0, when='AT_END')
#    mhe.constrain(mhe['ConstR6'],'==',0, when='AT_END')

    mhe.constrain(mhe['Const'],'==',0, when='AT_END')
    mhe.constrain(mhe['dConst'],'==',0, when='AT_END')

#    mhe.constrain(mhe['ConstDelta'],'==',0, when='AT_END')

    acadoOpts = [('HESSIAN_APPROXIMATION','GAUSS_NEWTON'),
                 ('DISCRETIZATION_TYPE','MULTIPLE_SHOOTING'),
                 ('QP_SOLVER','QP_QPOASES'),
                 #('HOTSTART_QP','YES'),
                 ('INTEGRATOR_TYPE',iType),
                 ('NUM_INTEGRATOR_STEPS',str(nSteps*N)),
                 ('SPARSE_QP_SOLUTION','CONDENSING'),
                 ('FIX_INITIAL_STATE','NO'),
#                 ('LEVENBERG_MARQUARDT', '1e-4'),
                 ('CG_USE_VARIABLE_WEIGHTING_MATRIX','NO'),
                 ]



    cgOpts = {'CXX':'g++', 'CC':'gcc'}
#    cgOpts = {'CXX':'clang++', 'CC':'clang'}
    mheRT = mhe.exportCode(codegenOptions=cgOpts,acadoOptions=acadoOpts)
    return mheRT


if __name__=='__main__':
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)
    dae = carouselModel.makeModel(dae,conf)
    OcpRt = makeMhe(dae,10,0.1)

