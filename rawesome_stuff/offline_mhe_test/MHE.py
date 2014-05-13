import casadi as C
import numpy as np
import rawe

import carouselModel
from rawe.models.arianne_conf import makeConf

# from common_conf import Ts

# Sampling time, aka 25 Hz
samplingTime = 0.04

# Horizon length
mheHorizonN = 15

mheIntOpts = rawe.RtIntegratorOptions()
mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_GL2'
# mheIntOpts['INTEGRATOR_TYPE'] = 'INT_IRK_RIIA3'
mheIntOpts['NUM_INTEGRATOR_STEPS'] = 2
# mheIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 5
# mheIntOpts['IMPLICIT_INTEGRATOR_NUM_ITS_INIT'] = 0
# mheIntOpts['LINEAR_ALGEBRA_SOLVER'] = 'HOUSEHOLDER_QR'
# mheIntOpts['UNROLL_LINEAR_SOLVER'] = False
# mheIntOpts['IMPLICIT_INTEGRATOR_MODE'] = 'IFTR'

mheOpts = rawe.OcpExportOptions()
mheOpts['HESSIAN_APPROXIMATION'] = 'GAUSS_NEWTON'
mheOpts['DISCRETIZATION_TYPE'] = 'MULTIPLE_SHOOTING'
mheOpts['QP_SOLVER'] = 'QP_QPOASES'
mheOpts['HOTSTART_QP'] = True
mheOpts['SPARSE_QP_SOLUTION'] = 'CONDENSING'
# mheOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING_U2'
# mheOpts['MAX_NUM_QP_ITERATIONS'] = '30'
mheOpts['FIX_INITIAL_STATE'] = False

# This is somehow mandatory, cause we use it on the real system (always!)
mheOpts['CG_USE_VARIABLE_WEIGHTING_MATRIX'] = True

# mheOpts['CG_CONDENSED_HESSIAN_CHOLESKY'] = 'INTERNAL_N3'

# And we introduce arrival cost computation
# mheOpts['CG_USE_ARRIVAL_COST'] = True

# Measurements depending only on diff. state variables

useLedTracker    = False
useLineAngleData = True
useVirtualTorques = True
useVirtualForces = False

measX  = ['cos_delta', 'sin_delta', 'IMU_angular_velocity', 'IMU_acceleration']
measX += ['aileron', 'elevator']
measX += ['r', 'dr', 'ddr']

if useLedTracker is True:
    measX += ['marker_positions']
if useLineAngleData is True:
    measX += ['lineAngles']

# Measurements depending only on control variables
measU  = ['daileron', 'delevator', 'dmotor_torque', 'dddr']

if useVirtualTorques is True:
    measU += ['dt1_disturbance', 'dt2_disturbance', 'dt3_disturbance']
if useVirtualForces is True:
    measU += ['df1_disturbance', 'df2_disturbance', 'df3_disturbance']

#
# Standard deviations for measurements
#

# Mario's
# mheSigmas = {'cos_delta':1e-1, 'sin_delta':1e-1,
# 			   'IMU_angular_velocity':1.0,
# 			   'IMU_acceleration':10.0,
# 			   'marker_positions':1e2,
# 			   'aileron':1e-2,
# 			   'elevator':1e-2,
# 			   'daileron':1e-4,
# 			   'delevator':1e-4,
# 			   'dmotor_torque':1e-4,
# 			   'dddr':1e-4}

gyro_lsb = np.radians( 0.2 ) # 1 LSB of the gyroscope
accl_lsb = 3.33 / 1000.0 * 9.81 # 1 LSB of the accelerometer
servo_lsb = 2.0 * np.pi / 1024.0

# New, verified with experimental data
mheSigmas = {'cos_delta': 1e-3, 'sin_delta': 1e-3,

             'IMU_angular_velocity': 50.0 * gyro_lsb, 
             'IMU_acceleration': 50.0 * accl_lsb, 
             
             'aileron': servo_lsb, 'elevator': servo_lsb,
             'daileron': 1e0, 'delevator': 1e0,

             'r': 1e-1, 'dr': 1e-1, 'ddr': 1e-1, 'dddr': 1e-1,

             'dmotor_torque': 1e1
			 }

if useLedTracker is True:
    mheSigmas.update({'marker_positions': 2e1}) # pixels
if useLineAngleData is True:
    mheSigmas.update({'lineAngles': 2.0 * 4.0 * servo_lsb})
if useVirtualTorques is True:
    mheSigmas.update({'dt1_disturbance': 1e1, 'dt2_disturbance': 1e1, 'dt3_disturbance' : 1e1})
if useVirtualForces is True:
    mheSigmas.update({'df1_disturbance': 1e2, 'df2_disturbance': 1e2, 'df3_disturbance' : 1e2})

# Weights
mheWeightScaling = 1e-3

mheWeights = {}
for sigma in mheSigmas:
	mheWeights[ sigma ] = mheWeightScaling / mheSigmas[ sigma ] ** 2

def makeMhe(Ts = None, propertiesDir = '../../properties'):
	
	#
	# Make the parameter configuration for the model
	#
	conf = makeConf()
	conf[ 'stabilize_invariants' ] = False
	conf[ 'useVirtualTorques' ]    = True
	conf[ 'useVirtualForces' ]     = False 
	
	#
	# Create the DAE
	#
	dae = carouselModel.makeModel(conf, propertiesDir = propertiesDir)

	if Ts is None:
		execSamplingTime = samplingTime
	else:
		execSamplingTime = Ts

	mhe = rawe.Mhe(dae, N = mheHorizonN, ts = execSamplingTime, yxNames = measX, yuNames = measU)

# 	mheConstraintsWhen = 'AT_START'
	mheConstraintsWhen = 'AT_END'

	mhe.constrain(mhe['ConstR1'], '==', 0, when = mheConstraintsWhen)
	mhe.constrain(mhe['ConstR2'], '==', 0, when = mheConstraintsWhen)
	mhe.constrain(mhe['ConstR3'], '==', 0, when = mheConstraintsWhen)
	mhe.constrain(mhe['ConstR4'], '==', 0, when = mheConstraintsWhen)
	mhe.constrain(mhe['ConstR5'], '==', 0, when = mheConstraintsWhen)
	mhe.constrain(mhe['ConstR6'], '==', 0, when = mheConstraintsWhen)

	mhe.constrain(mhe['c'], '==', 0, when = mheConstraintsWhen)
	mhe.constrain(mhe['cdot'], '==', 0, when = mheConstraintsWhen)

	mhe.constrain(mhe['ConstDelta'], '==', 0, when = mheConstraintsWhen)

	return mhe

def makeMheRT(Ts, propertiesDir = '../../properties', cgOptions = None):
	if cgOptions is None:
		cgOptions = {'CXX':'clang++', 'CC':'clang',
					'CXXFLAGS':'-O3 -fPIC -finline-functions -march=native',
					'CFLAGS':'-O3 -fPIC -finline-functions -march=native'}
# 					  , 'force_export_path':'export_here'}

	# If we want to export to a specific part, we would need the following option:
	# 'export_without_build_path': '<path>'

	mhe = makeMhe(Ts = Ts, propertiesDir = propertiesDir)
	return rawe.MheRT(mhe, ocpOptions = mheOpts, integratorOptions = mheIntOpts, codegenOptions = cgOptions)
