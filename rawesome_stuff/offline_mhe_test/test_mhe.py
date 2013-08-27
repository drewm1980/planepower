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
data = sio.loadmat('dataset_20130729_190735_kmhe_timings.mat')

# Number of IMU measurements between two subsequent camera measurements
nDelay = 4

# Sampling time
Ts = 0.02 * nDelay

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
measStart = 4612
measEnd   = 23600
while cam_flag[measStart] != 1:
    measStart += 1

measurements = {}
measurements['time'] = time[measStart: measEnd: nDelay, :]
measurements['marker_positions'] = cam_led[measStart + nDelay: measEnd + nDelay: nDelay, :]

measurements['pfm_position'] = cam_pose[measStart + nDelay: measEnd + nDelay: nDelay, :3]
# TODO convert pfm_dcm data to 3x3 matrix
measurements['pfm_dcm']      = cam_pose[measStart + nDelay: measEnd + nDelay: nDelay, 3:]

#measurements['xyz'] = cam_pose[measStart+nDelay:measEnd+nDelay:nDelay,:]
measurements['cos_delta'] = enc_data[measStart: measEnd: nDelay, 2]
measurements['sin_delta'] = -enc_data[measStart: measEnd: nDelay, 1]

# For the IMU data there are two variants:

# 1. First measurements from the interval (k-1, k]
#measurements['IMU_angular_velocity'] = imu_first[measStart:measEnd:nDelay,:3]
#measurements['IMU_acceleration'] = imu_first[measStart:measEnd:nDelay,3:]

# 2. Averaged measurements on the interval (k-1, k]
measurements['IMU_angular_velocity'] = imu_avg[measStart: measEnd: nDelay, :3]
measurements['IMU_acceleration'] = imu_avg[measStart: measEnd: nDelay, 3:]

measurements['aileron'] = control_surf[measStart: measEnd: nDelay, 0]
measurements['elevator'] = control_surf[measStart: measEnd: nDelay, 2]

# This is fake, of course...
measurements['dddr'] = enc_data[measStart: measEnd: nDelay, 0] * 0

# Just for fun
delta = enc_data[measStart:,0]
ddelta = numpy.diff(delta)/float(time[1,0]-time[0,0])

for index, speed in enumerate(ddelta):
    if speed < 0: ddelta[index] = ddelta[index - 1]

plot_speed = 0
if plot_speed:    
	plt.figure()
	p, = plt.step(range( ddelta.shape[ 0 ] ), ddelta)
	plt.legend([p], ['speed [rad/s]'])

	plt.show()
	exit( 0 )

###############################################################################
#
# MHE solver configuration
#
###############################################################################

# Get the plane configuration parameters
conf = makeConf()
conf['stabilize_invariants'] = True
daeSim = carouselModel.makeModel(conf, propertiesDir = '../../properties')

# Create the MHE class
mheRT = MHE.makeMheRT(Ts = Ts)

# A list of measurement functions
measX = ['marker_positions', 'cos_delta', 'sin_delta', 'IMU_angular_velocity', 'IMU_acceleration']
measX += ['aileron', 'elevator']

measU = ['dddr']

#
# Standard deviations for the measurements
#
mheSigmas = {'cos_delta':1e-1, 'sin_delta':1e-1,
             'IMU_angular_velocity':1.0,
             'IMU_acceleration':10.0,
             'marker_positions':1e2,
             'r':1e-1,
             'aileron':1e-2,
             'elevator':1e-2,
             'daileron':1e-4,
             'dmotor_torque':1e-4,
             'delevator':1e-4,
             'dddr':1e-4}

#
# Define actual weights for the MHE
#

Weight_ = numpy.array([])
for name in mheRT.ocp.yxNames + mheRT.ocp.yuNames:
    Weight_ = numpy.append(Weight_, numpy.ones(mheRT.ocp.dae[name].shape) * (1.0 / mheSigmas[name] ** 2))
mheRT.S = numpy.diag(Weight_)

Weight_ = numpy.array([])
for name in mheRT.ocp.yxNames:
    Weight_ = numpy.append(Weight_, numpy.ones(mheRT.ocp.dae[name].shape) * (1.0 / mheSigmas[name] ** 2))
mheRT.SN = numpy.diag(Weight_)

###############################################################################
#
# Get the initial guess for the MHE by calculating a steady state
#
###############################################################################

# Reference parameters
refP = {'r0':1.2,
        'ddelta0':-ddelta[0],
        }

ref_dict = {
            'z':(-10,10),
            'aileron':(0,0),
            'elevator':(0,0),
            'rudder':(0,0),
            'flaps':(0,0),
            }
#ref_dict = {
#            'z':(0,0),
#            'aileron':(-1,1),
#            'elevator':(-1,1),
#            'rudder':(-1,1),
#            'flaps':(-1,1),
#            }

# Get the steady state
steadyState,dSS = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0'], ref_dict)

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
        numpy.array([mheRT.computeYU(mheRT.u[k,:])])
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
		numpy.array([mheRT.computeYU(mheRT.u[k, :])])
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
		numpy.concatenate([numpy.array([measurements[name][k]]).flatten() for name in measU])
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

###############################################################################
#
# After several light years we finally reached the code that runs the simulation
#
###############################################################################

print "Exiting because weighting matrices are different in the MHE.py."
print "Otherwise you would get an error"
exit( 0 )

print "\n\n*** Alright, now we start the simulation ***\n\n"

for k in range(12):
    if mheRT.y[0, k] < 0:
        mheRT.S[k, k] = 0
for k in range(12):
    mheRT.SN[k, k] = 0

# Number of simulations steps
nSim = 1
# Maximum number of SQP iterations
nSqp = 5
# Initialized flag
initialized = False

for itSim in range( nSim ):
	print "MHE simulation step: " + str(itSim + 1)
	
# 	y = numpy.zeros((mheRT.ocp.N, ny))
# 	
# 	for k in range(mheRT.ocp.N):
# 		y[k,:] = numpy.concatenate([
#                                     mheRT.computeYX( mheRT.x[k, :] ),
#                                     numpy.array([mheRT.computeYU( mheRT.u[k,:])])
#                                     ])
	
	if initialized == False:
		initialized = True
	else:
		# Shift states and controls
		mheRT.shiftXZU(strategy = 'simulate')
		
		# Shift measurements
# 		mheRT.simpleShiftReference(y_Nm1, yN)
	
	for itSqp in range( nSqp ):
		mheRT.preparationStep()
		mheRT.feedbackStep()
	
		print "  RTI %3d: {kkt: %.4e, obj: %.4e, exec. time: %2.0f [ms]}" \
			% (itSqp + 1, mheRT.getKKT(), mheRT.getObjective(),
				1e3 * (mheRT.preparationTime + mheRT.feedbackTime))
	
	mheRT.log()

exit( 0 )

###############################################################################
#
# Post processing: plotting of the simulation results
#
###############################################################################

# This is just a quick hack to make the class constructor happy
sim = rawe.RtIntegrator(daeSim, ts = Ts, options = MHE.mheIntOpts)

plotter = Plotter(sim, mheRT, sim)

plotter.subplot([['x','y','z'],['dx','dy','dz']],what=['mhe'])
plotter.subplot([['r'],['dr']],what=['mhe'])
plotter.subplot([['e11','e12','e13'],['e21','e22','e23'],['e31','e32','e33']],what=['mhe'])
plotter.subplot(['w_bn_b_x','w_bn_b_y','w_bn_b_z'],what=['mhe'])
plotter.subplot([['aileron','elevator'],['daileron','delevator']],what=['mhe'])
plotter.subplot([['cos_delta','sin_delta'],['ddelta'],['motor_torque']],what=['mhe'])
plotter.subplot([['r'],['dr'],['ddr']],what=['mhe'])
plotter.subplot([['c'],['cdot']],what=['mhe'])
plotter.subplot([['kkt'],['_objective'],['_prep_time','_fb_time']],what=['mhe'])
# plotter.plot(['_kkt'],what=['mhe'])
# plotter.plot(['tether_tension'],what=['mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['mhe'])
# plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim'])
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['mhe'])
# plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim'])
# plotter.plot(['fL'],what=['mhe'])
# plotter.plot(['aero_mx'],what=['mhe'])

plt.show()
