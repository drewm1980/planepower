#!/usr/bin/env python

import numpy
import matplotlib.pyplot as plt
import scipy.io as sio

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
#data = sio.loadmat('dataset_20130819_190802_kmhe_timings.mat')

#
# Data sets with extended line length
#

# Cold start
# data = sio.loadmat('dataset_20130827_130807_kmhe_timings.mat')

# Warmed up
#data = sio.loadmat('dataset_20130827_130802_kmhe_timings.mat')

# #samples, representing camera delay
#nDelay = 4

#
# New data set @ 25 Hz
#
#data = sio.loadmat('dataset_20130831_200853_kmhe_timings.mat')
data = sio.loadmat('dataset_20140206_185035_dmhe_testing.mat')

data = sio.loadmat('dataset_20140403_160157_dmhe_testing.mat')

# #samples, representing camera delay
nDelay = 2

# Sampling time, 50 Hz as in the datasets
Ts = 0.04

# Read data to temporary data structures
time         = numpy.array(data['dataset'][0,0]['time'])
imu_first    = numpy.array(data['dataset'][0,0]['imu_first'])
imu_avg      = numpy.array(data['dataset'][0,0]['imu_avg'])
enc_data     = numpy.array(data['dataset'][0,0]['enc_data'])
cam_led      = numpy.array(data['dataset'][0,0]['cam_led'])
cam_pose     = numpy.array(data['dataset'][0,0]['cam_pose'])
cam_flag     = numpy.array(data['dataset'][0,0]['cam_flag'])
control_surf = numpy.array(data['dataset'][0,0]['controls'])

R_nwu2ned = numpy.eye( 3 )
R_nwu2ned[1, 1] = R_nwu2ned[2, 2] = -1.0

cam_xyz = numpy.zeros([time.shape[ 0 ], 3])
for k in range( time.shape[ 0 ] ):
	cam_xyz[k, :] = numpy.dot(R_nwu2ned, cam_pose[k, :3].T)

cam_rpy = numpy.zeros([time.shape[ 0 ], 3])	
for k in range( time.shape[ 0 ] ):
	R = cam_pose[k, 3:].reshape((3, 3))
	
	yaw = numpy.arctan2(R[0, 1], R[0, 0]) * cam_flag[ k ]
	pitch = numpy.arcsin(-R[0, 2]) * cam_flag[ k ]
	roll = numpy.arctan2(R[1, 2], R[2, 2])  * cam_flag[ k ]
	
	cam_rpy[k, :] = [roll, pitch, yaw]
	cam_rpy[k, :] *= 180 / numpy.pi

logR = numpy.zeros( cam_xyz.shape[ 0 ] )
for k in range(cam_xyz.shape[ 0 ]):
	logR[ k ] = numpy.linalg.norm( cam_xyz[k, :] )
	
cam_time = time - Ts * nDelay

print "Printing plots..."
	
plt.figure()
plt.plot(logR * cam_flag[ k ], 'or') 

plt.figure()
plt.subplot(3, 1, 1)
plt.plot(cam_rpy[:, 0], 'or')
plt.subplot(3, 1, 2)
plt.plot(cam_rpy[:, 1], 'og')
plt.subplot(3, 1, 3)
plt.plot(cam_rpy[:, 2], 'ob')

plt.figure()
plt.subplot(3, 1, 1)
plt.plot( control_surf[:, 0] )
plt.subplot(3, 1, 2)
plt.plot( control_surf[:, 1] )
plt.subplot(3, 1, 3)
plt.plot( control_surf[:, 2] )

plt.show()
