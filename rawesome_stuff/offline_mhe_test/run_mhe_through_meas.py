#!/usr/bin/env python

import numpy
import matplotlib.pyplot as plt
import scipy.io as sio

from sys import exit

# Rawesome deps
import rawe
from rawe.models.arianne_conf import makeConf
from rawekite.carouselSteadyState import getSteadyState
from mpc_mhe_utils import Plotter

# Local imports from this folder
import MHE
import carouselModel

###############################################################################
#
# Load measurement data from a file
#
###############################################################################

# A really long data set, carousel accelerating from low speed.
# Ts = 1 / 50 = 0.02 [ms], camera data sample rate is 0.08 [ms]
#data = scipy.io.loadmat('dataset_20130729_180717_kmhe_timings.mat')

# Another bit shorter data set, without a gap of camera data in the beginning,
# here the carousel is warmed-up already.
# Ts = 1 / 50 = 0.02 [ms], camera data sample rate is 0.08 [ms]
# data = sio.loadmat('dataset_20130729_190735_kmhe_timings.mat')

#
# New data sets, where cameras work all the time
#

# Cold start
# data = sio.loadmat('dataset_20130819_180839_kmhe_timings.mat')

# Warmed up
# data = sio.loadmat('dataset_20130819_190802_kmhe_timings.mat')

#
# Data sets with extended line length
#

# Cold start
# data = sio.loadmat('dataset_20130827_130807_kmhe_timings.mat')

# Warmed up
data = sio.loadmat('dataset_20130827_130802_kmhe_timings.mat')

# #samples, representing camera delay
nDelay = 4

# Sampling time, 50 Hz as in the datasets
Ts = 0.02

# Read data to temporary data structures
time         = data['dataset'][0,0]['time']
imu_first    = data['dataset'][0,0]['imu_first']
imu_avg      = data['dataset'][0,0]['imu_avg']
enc_data     = data['dataset'][0,0]['enc_data']
cam_led      = data['dataset'][0,0]['cam_led']
cam_pose     = data['dataset'][0,0]['cam_pose']
cam_flag     = data['dataset'][0,0]['cam_flag']
las_data     = data['dataset'][0,0]['las_data']
control_surf = data['dataset'][0,0]['controls']

#
# Trim the data set
#
# measStart = 4612
# measEnd   = 23600

# The new one, where all LEDs work
# measStart = 1000
# measEnd   = 3000

#
# The new data set with extended line length
#
measStart = 3100 + 2000
measEnd   = measStart + 1000

while cam_flag[measStart] != 1:
	measStart += 1

measurements = {}
measurements['time'] = time[measStart: measEnd, :]
measurements['marker_positions'] = cam_led[measStart: measEnd, :]

measurements['cam_flag'] = cam_flag[measStart: measEnd, :]

measurements['pfm_position'] = cam_pose[measStart: measEnd, :3]
# TODO convert pfm_dcm data to 3x3 matrix
measurements['pfm_dcm']      = cam_pose[measStart: measEnd, 3:]

#measurements['xyz'] = cam_pose[measStart+nDelay:measEnd+nDelay:nDelay,:]
measurements['cos_delta'] = enc_data[measStart: measEnd, 2]
measurements['sin_delta'] = -enc_data[measStart: measEnd, 1]

# For the IMU data there are two variants:

# 1. First measurements from the interval (k-1, k]
#measurements['IMU_angular_velocity'] = imu_first[measStart:measEnd:nDelay,:3]
#measurements['IMU_acceleration'] = imu_first[measStart:measEnd:nDelay,3:]

# 2. Averaged measurements on the interval (k-1, k]
measurements['IMU_angular_velocity'] = imu_avg[measStart: measEnd, :3]
measurements['IMU_acceleration'] = imu_avg[measStart: measEnd, 3:]

measurements['aileron'] = control_surf[measStart: measEnd, 0]
measurements['elevator'] = control_surf[measStart: measEnd, 2]

# FAKE!
measurements['r'] = control_surf[measStart: measEnd, 2] * 0
measurements['dr'] = control_surf[measStart: measEnd, 2] * 0
measurements['ddr'] = control_surf[measStart: measEnd, 2] * 0

#
# Prepare xyz from markers
#
R_nwu2ned = numpy.eye( 3 )
R_nwu2ned[1, 1] = R_nwu2ned[2, 2] = -1.0

cam_xyz = numpy.zeros([measurements['pfm_position'].shape[ 0 ], 3])
for k in range( measurements['pfm_position'].shape[ 0 ] ):
	cam_xyz[k, :] = numpy.dot(R_nwu2ned, measurements['pfm_position'][k, :].T)
	
cam_time = numpy.array(measurements['time']) - nDelay * Ts

#
# Some sanity checks
#

delta = enc_data[measStart: measEnd, 0]
ddelta = numpy.diff(delta) / float(time[1, 0] - time[0, 0])

plot_meas = 0
if plot_meas: 
	for index, speed in enumerate(ddelta):
		if speed < 0: ddelta[index] = ddelta[index - 1]
	
	plt.figure()
	p, = plt.step(range( ddelta.shape[ 0 ] ), ddelta)
	plt.legend([p], ['speed [rad/s]'])
	
	plt.figure()
	p, = plt.step(time[measStart: measEnd], cam_led[measStart: measEnd, 0])
	plt.legend([p], ['cam data'])

	plt.show()
	exit( 0 )

###############################################################################
#
# MHE solver configuration
#
###############################################################################

# Get the plane configuration parameters
conf = makeConf()
# conf['stabilize_invariants'] = True
daeSim = carouselModel.makeModel(conf, propertiesDir = '../../properties')

# Create the MHE class
mheRT = MHE.makeMheRT(Ts = Ts)

# A list of measurement functions
measX = MHE.measX
measU = MHE.measU

###############################################################################
#
# Get the initial guess for the MHE by calculating a steady state
#
###############################################################################

#
# Cable length which we are going to supply to the MHE as a fake measurement
#
measCableLength = 2.0

#
# Speed for the steady state calculation
#
steadyStateSpeed = -4.0
#steadyStateSpeed = -ddelta[ 0 ]

# Reference parameters
refP = {'r0': measCableLength,
		'ddelta0': steadyStateSpeed,
        }

# Latest
#ref_dict = {
#            'z':(-10,10),
#            'aileron':(0,0),
#            'elevator':(0,0),
#            'rudder':(0,0),
#            'flaps':(0,0),
#            }
#ref_dict = {
#            'z':(0,0),
#            'aileron':(-1,1),
#            'elevator':(-1,1),
#            'rudder':(-1,1),
#            'flaps':(-1,1),
#            }

# Get the steady state
steadyState,dSS = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0'])

# Utility functions
def getDeltaRange(delta0, kRange):
    return numpy.array([delta0 + k*Ts*refP['ddelta0'] for k in kRange])
def getDeltaRangeMhe(delta0):
    return getDeltaRange(delta0, range(-MHE.mheHorizonN, 1))

# Starting angle, from measurements
delta0 = numpy.arctan2(measurements['sin_delta'][0], measurements['cos_delta'][0])

# Initialize differential state variables
for k, name in enumerate(mheRT.ocp.dae.xNames()):
    mheRT.x[:, k] = steadyState[name]
    
    # Rewrite angles again? Now with actual angles from measurements? 
    if name == 'sin_delta':
        mheRT.x[:, k] = numpy.sin(getDeltaRange(delta0, range(MHE.mheHorizonN + 1)))
    if name == 'cos_delta':
        mheRT.x[:, k] = numpy.cos(getDeltaRange(delta0, range(MHE.mheHorizonN + 1)))

# Initialize algebraic variables
for k, name in enumerate(mheRT.ocp.dae.zNames()):
    mheRT.z[:, k] = steadyState[name]
    
# Initialize controls
for k, name in enumerate(mheRT.ocp.dae.uNames()):
    mheRT.u[:, k] = steadyState[name]

###############################################################################
#
# Sanity check of measurement functions
#
###############################################################################

# Number of measurements for nodes 0..N - 1
ny = numpy.concatenate([
		mheRT.computeYX(mheRT.x[k,:]),
        mheRT.computeYU(mheRT.u[k,:])
        ]).shape[0]

# Initialize vector of measurements
y = numpy.zeros((mheRT.ocp.N, ny))

#
# Calculate expected measurements
#

# ... for nodes 0..N - 1
for k in range(mheRT.ocp.N):
    y[k, :] = numpy.concatenate([
		mheRT.computeYX(mheRT.x[k, :]),
		mheRT.computeYU(mheRT.u[k, :])
		])
# ... and for the last node
yN = mheRT.computeYX(mheRT.x[-1, :])

#
# And here are the real measurements
#

# ... for nodes 0..N - 1
for k in range(mheRT.ocp.N):
    mheRT.y[k, :] = numpy.concatenate([
		numpy.concatenate([numpy.array([measurements[name][k]]).flatten() for name in measX]),
		numpy.zeros([1, len( measU )]).flatten()
		])
# ... and for the last node
mheRT.yN = numpy.concatenate([
		numpy.array([measurements[name][mheRT.ocp.N]]).flatten() for name in measX
		])

#
# Plotting
#
def plot(y, y_hat):

    plt.figure()
    plt.subplot(2, 2, 1)
    plt.plot([0, 1200], [0, 1200], linewidth = 0)
    color = ['b', 'r', 'k']
    for k in range(3):
        plt.plot(y[:, 2 * k], y[:, 2 * k + 1], 'x' + color[k])
        plt.plot(y_hat[:, 2 * k], y_hat[:, 2 * k + 1], '.' + color[k])
    plt.grid('on')
    plt.subplot(2, 2, 2)
    plt.plot([0, 1200], [0, 1200], linewidth = 0)
    for k in range(3):
        plt.plot(y[:, 2 * k + 6], y[:, 2 * k + 7], 'x' + color[k])
        plt.plot(y_hat[:, 2 * k + 6], y_hat[:, 2 * k + 7], '.' + color[k])
    plt.grid('on')
    
    plt.figure()
    plt.subplot(2, 1, 1)
    color = ['b', 'r', 'k']
    for k in range(3):
        plt.plot(y[:, k + 14], 'x' + color[k])
        plt.plot(y_hat[:, k + 14], '.' + color[k])
    plt.grid('on')
    plt.subplot(2, 1, 2)
    for k in range(3):
        plt.plot(y[:, k + 17], 'x' + color[k])
        plt.plot(y_hat[:, k + 17], '.' + color[k])
    plt.grid('on')
    
    plt.figure()
    for k in range(2):
        plt.plot(y[:, k + 12], 'x' + color[k])
        plt.plot(y_hat[:, k + 12], '.' + color[k])
    plt.grid('on')
    
    plt.figure()
    for k in range(2):
        plt.plot(y[:, k + 20], 'x' + color[k])
        plt.plot(y_hat[:, k + 20], '.' + color[k])
    plt.grid('on')

# Sanity check for measurement data
plot_sanity_check = 0
if plot_sanity_check:
	plot(y, mheRT.y)
	plt.show()
	exit( 0 )

###############################################################################
#
# Simulation visualisation stuff
#
###############################################################################

fig1 = plt.figure()
fig1.suptitle("Sine and cos of the carousel angle.")
sinCosPlt = fig1.add_subplot(111)

fig2 = plt.figure()
fig2.suptitle("Gyroscope measurements.")
gyroPlt = fig2.add_subplot(111)

fig3 = plt.figure()
fig3.suptitle("Accelerometers measurements.")
acclPlt = fig3.add_subplot(111)

ledPlt = []
fig4 = plt.figure()
fig4.suptitle("Camera measurements.")
for row in range( 2 ):
	for col in range( 6 ):
		ledPlt.append(fig4.add_subplot(2, 6, row * 6 + col + 1))
		
def visualize(projY, projYN, measY, measYN):
	
	def joinStuff(a, b):
		return numpy.concatenate((a.flatten(), numpy.array([b])))
	
	# Plot sin and cos of the carousel angle
 	sinCosPlt.clear()
 	sinCosPlt.plot(joinStuff(projY[:, 12], projYN[ 12 ]), '-ob')
 	sinCosPlt.plot(joinStuff(measY[:, 12], measYN[ 12 ]), '-xb')
 			
 	sinCosPlt.plot(joinStuff(projY[:, 13], projYN[ 13 ]), '-og')
 	sinCosPlt.plot(joinStuff(measY[:, 13], measYN[ 13 ]), '-xg')
 			
 	fig1.canvas.draw()
 			
 	gyroPlt.clear()
 			
 	gyroPlt.plot(joinStuff(projY[:, 14], projYN[ 14 ]), '-ob')
 	gyroPlt.plot(joinStuff(measY[:, 14], measYN[ 14 ]), '-xb')
 			
 	gyroPlt.plot(joinStuff(projY[:, 15], projYN[ 15 ]), '-og')
 	gyroPlt.plot(joinStuff(measY[:, 15], measYN[ 15 ]), '-xg')
 			
 	gyroPlt.plot(joinStuff(projY[:, 16], projYN[ 16 ]), '-or')
 	gyroPlt.plot(joinStuff(measY[:, 16], measYN[ 16 ]), '-xr')
 			
 	fig2.canvas.draw()
 			
 	acclPlt.clear()
 			
 	acclPlt.plot(joinStuff(projY[:, 17], projYN[ 17 ]), '-ob')
 	acclPlt.plot(joinStuff(measY[:, 17], measYN[ 17 ]), '-xb')
 		
 	acclPlt.plot(joinStuff(projY[:, 18], projYN[ 18 ]), '-og')
  	acclPlt.plot(joinStuff(measY[:, 18], measYN[ 18 ]), '-xg')
  			
  	acclPlt.plot(joinStuff(projY[:, 19], projYN[ 19 ]), '-or')
  	acclPlt.plot(joinStuff(measY[:, 19], measYN[ 19 ]), '-xr')
  			
  	fig3.canvas.draw()
  			
   	for ind in range( 12 ):
   		ledPlt[ ind ].clear()
   		ledPlt[ ind ].plot(joinStuff(projY[:, ind], projYN[ ind ]), 'or')
   		ledPlt[ ind ].plot(joinStuff(measY[:, ind], measYN[ ind ]), 'xr')
   	
  	fig4.canvas.draw()
 			
	raw_input("Press enter to continue")

###############################################################################
#
# After several light years we finally reached the code that runs the simulation
#
###############################################################################

print "\n\n*** Alright, now we start the simulation ***\n\n"

#
# Define actual weights for the MHE
#

diagWeightNY = numpy.array([])
for name in mheRT.ocp.yxNames + mheRT.ocp.yuNames:
	diagWeightNY = numpy.append(diagWeightNY, 
			numpy.ones(mheRT.ocp.dae[name].shape) * MHE.mheWeights[ name ])
mheNY = diagWeightNY.shape[ 0 ]

diagWeightNYN = numpy.array([])
for name in mheRT.ocp.yxNames:
	diagWeightNYN = numpy.append(diagWeightNYN,
			numpy.ones(mheRT.ocp.dae[name].shape) * MHE.mheWeights[ name ])
mheNYN = diagWeightNYN.shape[ 0 ]

mheRT.y = numpy.zeros(mheRT.y.shape)
mheRT.yN = numpy.zeros(mheRT.yN.shape)

defaultDiagWeightNY = numpy.array(diagWeightNY)
defaultDiagWeightNY[ :12 ] *= 0
for n in range( MHE.mheHorizonN ):
	mheRT.S[n * mheNY: (n + 1) * mheNY, :] = numpy.diag( defaultDiagWeightNY )

# There is no camera data on the last node
# Thus this a TODO: remove camera measurements.
diagWeightNYN[ :12 ] *= 0
mheRT.SN = numpy.diag( diagWeightNYN )

#
# Try initialization by a forward simulation
#
# mheRT.initializeNodesByForwardSimulation()

#
# Visualisation
#
visualise_steps = False

#
# Arrival cost settings
#

use_arrival_cost = 0

if use_arrival_cost:
	mheRT.xAC = mheRT.x[0, :]
	mheRT.SAC = 1e-2 * numpy.eye(mheRT.x[0, :].shape[ 0 ])
	mheRT.WL  = 1e2 * numpy.eye(mheRT.x[0, :].shape[ 0 ])

# Number of simulations steps
nSim = MHE.mheHorizonN + 350
# Maximum number of SQP iterations
nSqp = 1
# Initialized flag
initialized = False

scY = numpy.zeros(mheRT.y.shape)

plt.ion()
plt.show()

for itSim in range( nSim ):
	print "MHE simulation step: " + str(itSim + 1)
	
	#
	# Alright, fill the estimator with measurements
	#
	
	led_data = numpy.array(measurements['marker_positions'][ itSim ])
	
# 	if itSim < 100:
	led_data_valid = (led_data > 0) * measurements['cam_flag'][ itSim ]
# 	else:
# 		led_data_valid = 0
	
 	led_data *= led_data_valid
 	
 	tmpS = numpy.copy( diagWeightNY )
 	tmpS[ :12 ] *= led_data_valid	
 	tmpS = numpy.diag( tmpS )
 	
 	if measurements['cam_flag'][ itSim ]:
 		print "\n\n *** Hooray, new LED data arrived!!! *** \n\n"
 	
	y = numpy.concatenate([
			numpy.zeros( led_data.shape ),
			numpy.array(measurements['cos_delta'][ itSim ]).flatten(),
			numpy.array(measurements['sin_delta'][ itSim ]).flatten(),
			numpy.array(measurements['IMU_angular_velocity'][ itSim ]).flatten(),
			numpy.array(measurements['IMU_acceleration'][ itSim ]).flatten(),
			numpy.array(measurements['aileron'][ itSim ]).flatten(),
			numpy.array(measurements['elevator'][ itSim ]).flatten(),
			])
	# Append r, dr, ddr
	y = numpy.append(y, numpy.array([measCableLength, 0, 0]).flatten())
	yN = y
	# Append controls which we actually don't know
	y = numpy.append(y, numpy.zeros([1, len( measU )]).flatten())
	
   	if itSim < MHE.mheHorizonN:
   		# Just fill in the measurements
   		
   		mheRT.y[itSim, :] = y
   		# TODO Here we might need to put different init for u's
   	
   		ledInd = itSim - nDelay	
   		if ledInd >= 0:
   			mheRT.y[ledInd, :12] = led_data
   			mheRT.S[ledInd * mheNY: (ledInd + 1) * mheNY, :]  = tmpS
   		
 		runMhe = 0
 	
 	elif itSim == MHE.mheHorizonN:
 		# Initialize
 		
 		mheRT.yN = yN
 		
 		ledInd = itSim - nDelay
 		mheRT.y[ledInd, :12] = led_data
   		mheRT.S[ledInd * mheNY: (ledInd + 1) * mheNY, :]  = tmpS
   		
   		# Initialize arrival cost
   		if use_arrival_cost:
   			mheRT.updateArrivalCost( True )
   			
#    		print "xAC"
# 		print mheRT.xAC
# 		print "SAC"
# 		print mheRT.SAC
   		
   		runMhe = 1
 		
 	else:
 		# Shift weighting matrices S_{0..N - 1}
 		# We do not need to move SN -> last S block since the camera delay is > 1
 		for n in range(MHE.mheHorizonN - 1):
 			mheRT.S[n * mheNY: (n + 1) * mheNY, :] = mheRT.S[(n + 1) * mheNY: (n + 2) * mheNY, :]
 		
 		# Shift measurements
 		for n in range(MHE.mheHorizonN - 1):
 			mheRT.y[n, :] = mheRT.y[n + 1, :]
 		mheRT.y[-1, :mheNYN] = mheRT.yN
 		
 		#
 		# Put new measurements
 		#
 		
 		mheRT.yN = yN
 		
 		ledInd = MHE.mheHorizonN - nDelay
 		mheRT.y[ledInd, :12] = led_data
   		mheRT.S[ledInd * mheNY: (ledInd + 1) * mheNY, :]  = tmpS
   		
   		#
   		# Shift states and controls
   		#
   		mheRT.shiftXZU(strategy = 'simulate')
   		
   		runMhe = 1
	
	mheFailed = False
	if runMhe:
		for itSqp in range( nSqp ):
			try:
				mheRT.preparationStep()
				mheRT.feedbackStep()
			except Exception as e:
				print e
			
				print "x"
				print mheRT.x
				print "u"
				print mheRT.u
				
				mheFailed = True
# 				print "xAC"
# 				print mheRT.xAC
			
			print "  RTI %3d: {kkt: %.4e, obj: %.4e, exec. time: %2.0f [ms]}" \
				% (itSqp + 1, mheRT.getKKT(), mheRT.getObjective(),
					1e3 * (mheRT.preparationTime + mheRT.feedbackTime))
				
			#
			# Plot measurements and data
			#
			
			
			#
			# Visualise current step
			#
			if visualise_steps:
				for k in range(mheRT.ocp.N):
					scY[k, :] = numpy.concatenate([
						mheRT.computeYX(mheRT.x[k, :]),
						mheRT.computeYU(mheRT.u[k, :])
						])
				scYN = mheRT.computeYX(mheRT.x[-1, :])
				
				visualize(scY, scYN, mheRT.y, mheRT.yN)
			
			#
 			# Update arrival cost for the next run
 			#
			if use_arrival_cost:
 				mheRT.updateArrivalCost( False )
 				
 			if mheFailed == True:
 				break
		
		mheRT.log()
	
	if mheFailed == True:
		break;

mhe_sanity_check = 0
if mhe_sanity_check:
	
	# Check sparsity patterns of the measurements and weights
 	plt.figure()
 	plt.spy(mheRT.y, markersize=5)
 	plt.figure()
 	plt.spy(mheRT.S, markersize=5)

	# ... for nodes 0..N - 1
	scY = numpy.zeros(mheRT.y.shape)
	for k in range(mheRT.ocp.N):
		scY[k, :] = numpy.concatenate([
						mheRT.computeYX(mheRT.x[k, :]),
						mheRT.computeYU(mheRT.u[k, :])
						])
	# ... and for the last node
	scYN = mheRT.computeYX(mheRT.x[-1, :])

	for n in  range( MHE.mheHorizonN ):
		print "Residuals y, row " + str( n )
		print (scY - mheRT.y)[n, :]
		print "\n\n"

	print "Residuals yN"
	print (scYN - mheRT.yN)
	print "\n\n"
	
	# Plot accelerations
	plt.figure()
	plt.subplot(2, 1, 1)
	plt.plot(scY[:, 17: 20])
	plt.subplot(2, 1, 2)
	plt.plot(mheRT.y[:, 17: 20])
	plt.show()

	exit( 0 )

###############################################################################
#
# Post processing: plotting of the simulation results
#
###############################################################################

plt.ioff()

# This is just a quick hack to make the class constructor happy
sim = rawe.RtIntegrator(daeSim, ts = Ts, options = MHE.mheIntOpts)

plotter = Plotter(sim, mheRT, sim)

plotter.subplot([['x', 'y', 'z'], ['dx', 'dy', 'dz']], what = ['mhe'])
plotter.subplot([['e11', 'e12', 'e13'], ['e21', 'e22', 'e23'], ['e31', 'e32', 'e33']], what = ['mhe'])
plotter.subplot(['w_bn_b_x', 'w_bn_b_y', 'w_bn_b_z'], what = ['mhe'])
 
plotter.subplot([['aileron', 'elevator'], ['daileron', 'delevator']], what = ['mhe'])
plotter.subplot([['cos_delta', 'sin_delta'], ['ddelta'], ['motor_torque']], what = ['mhe'])
  
plotter.subplot([['r'], ['dr'], ['ddr'], ['dddr']], what = ['mhe'])
  
plotter.subplot([['c'], ['cdot'], ['ConstDelta']], what = ['mhe'])
plotter.plot(['ConstR1', 'ConstR2', 'ConstR3', 'ConstR4', 'ConstR5', 'ConstR6'], what = ['mhe'])
plotter.subplot([['_kkt'], ['_objective'], ['_prep_time', '_fb_time']], what = ['mhe'])

N = MHE.mheHorizonN
plt.figure()
plt.plot(cam_time[N: nSim], cam_xyz[N: nSim, 0], 'ob')
plt.plot(cam_time[N: nSim], cam_xyz[N: nSim, 1], 'og')
plt.plot(cam_time[N: nSim], cam_xyz[N: nSim, 2], 'or')

def getLogLast(name):
	return [x[-1] for x in mheRT.getLog(name)]

plt.figure()
plt.subplot(2, 1, 1)
plt.plot(getLogLast('aileron'), 'ob')
plt.plot(getLogLast('elevator'), 'or')
plt.subplot(2, 1, 2)
plt.plot(measurements['aileron'][N: nSim], 'xb')

plt.figure()
plt.plot(control_surf)

plt.show()
