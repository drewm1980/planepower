import casadi as C
import rawe

import carouselModel
from rawe.models.arianne_conf import makeConf

#from common_conf import Ts

# Sampling time, aka 50 Hz
samplingTime = 0.04

# Horizon length
mheHorizonN = 20

mheIntOpts = rawe.RtIntegratorOptions()
mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_GL2'
#mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_RIIA3'
mheIntOpts['NUM_INTEGRATOR_STEPS'] = 2
# mheIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 5
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

# This is somehow mandatory, cause we use it on the real system (always!)
mheOpts['CG_USE_VARIABLE_WEIGHTING_MATRIX'] = True

# And we introduce arrival cost computation
# mheOpts['CG_USE_ARRIVAL_COST'] = True

# Measurements depending only on diff. state variables
# DO NOT CHANGE THIS ORDER
measX = ['marker_positions', 'cos_delta', 'sin_delta', 'IMU_angular_velocity', 'IMU_acceleration']
measX += ['aileron', 'elevator']
measX += ['r', 'dr', 'ddr']

# Measurements depending only on control variables
measU = ['daileron', 'delevator', 'dmotor_torque', 'dddr']

#
# Standard deviations for measurements
#

# Mario's
# mheSigmas = {'cos_delta':1e-1, 'sin_delta':1e-1,
#              'IMU_angular_velocity':1.0,
#              'IMU_acceleration':10.0,
#              'marker_positions':1e2,
#              'aileron':1e-2,
#              'elevator':1e-2,
#              'daileron':1e-4,
#              'delevator':1e-4,
#              'dmotor_torque':1e-4,
#              'dddr':1e-4}

# New, verified with experimental data
# DO NOT change the order !!!
mheSigmas = {
				'marker_positions': 5e0,
				
				'cos_delta': 1e-2, 'sin_delta': 1e-2,
				
				'IMU_angular_velocity': 1e-2,
				'IMU_acceleration': 1e-2,
             
             	'aileron': 1e-2,
			 	'elevator': 1e-2,
             
             	'r': 1e-2, 'dr': 1e-2, 'ddr': 1e-2,
			 	
			 	'daileron': 1e-2, 'delevator': 1e-2,
             
             	'dmotor_torque': 1e0,
             
			 	'dddr': 1e-4
			 }

# Weights
mheWeightScaling = 1e-3

mheWeights = {}
for sigma in mheSigmas:
	mheWeights[ sigma ] = mheWeightScaling / mheSigmas[ sigma ] ** 2

def makeMhe(Ts = None, propertiesDir = '../../properties', ):
    conf = makeConf()
    conf['stabilize_invariants'] = False
    dae = carouselModel.makeModel(conf, propertiesDir = propertiesDir)
    
    if Ts is None:
    	execSamplingTime = samplingTime
    else:
		execSamplingTime = Ts

    mhe = rawe.Mhe(dae, N = mheHorizonN, ts = execSamplingTime, yxNames = measX, yuNames = measU)
    
    mheConstraintsWhen = 'AT_START'
#     mheConstraintsWhen = 'AT_END'

    mhe.constrain(mhe['ConstR1'], '==', 0, when=mheConstraintsWhen)
    mhe.constrain(mhe['ConstR2'], '==', 0, when=mheConstraintsWhen)
    mhe.constrain(mhe['ConstR3'], '==', 0, when=mheConstraintsWhen)
    mhe.constrain(mhe['ConstR4'], '==', 0, when=mheConstraintsWhen)
    mhe.constrain(mhe['ConstR5'], '==', 0, when=mheConstraintsWhen)
    mhe.constrain(mhe['ConstR6'], '==', 0, when=mheConstraintsWhen)

    mhe.constrain(mhe['c'], '==', 0, when=mheConstraintsWhen)
    mhe.constrain(mhe['cdot'], '==', 0, when=mheConstraintsWhen)

    mhe.constrain(mhe['ConstDelta'],'==',0, when=mheConstraintsWhen)
    
#     mhe.constrain(mhe['r'], '==', 1.2, when='AT_START')
#     mhe.constrain(mhe['r'], '>=', 0.0, when='AT_START')
#     mhe.constrain(mhe['dr'], '==', 0.0, when='AT_START')
#     mhe.constrain(mhe['ddr'], '==', 0.0, when='AT_START')
#     mhe.constrain(mhe['dddr'], '==', 0.0)

    return mhe

def makeMheRT(Ts=0.1,propertiesDir='../../properties', cgOptions = None):
    if cgOptions is None:
        cgOptions= {'CXX':'clang++', 'CC':'clang',
                    'CXXFLAGS':'-O3 -fPIC -finline-functions -march=native',
                    'CFLAGS':'-O3 -fPIC -finline-functions -march=native'}
#                     , 'force_export_path':'export_here'}
    mhe = makeMhe(Ts=Ts,propertiesDir=propertiesDir)
    return rawe.MheRT(mhe, ocpOptions=mheOpts, integratorOptions=mheIntOpts, codegenOptions=cgOptions)

def milan_check_this_out():
    mhe = makeMhe()
    cgOptions= {'CXX':'clang++', 'CC':'clang',
                'CXXFLAGS':'-O3 -fPIC -finline-functions',
                'CFLAGS':'-O3 -fPIC -finline-functions',
                'export_without_build_path':'so_cool_i_do_it_myself'}
    phase1Options = {}
    mhe.exportCode(mheOpts, mheIntOpts, cgOptions, phase1Options)

if __name__ == '__main__':
    milan_check_this_out()
