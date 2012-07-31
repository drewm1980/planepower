#!/usr/bin/env python
from pose_from_markers import pose_and_covrt_from_markers
import numpy
from numpy import loadtxt
import pylab
from pylab import plot, xlabel, ylabel, axis, title, figure, show, legend, subplot
from rotation_matrices import roll_pitch_yaw_from_R
from roll_controller import get_rpy_from_pose

def filter_out_rows_containing_nan(A):
	return numpy.vstack([A[i,:] for i in xrange(A.shape[0]) if not any(numpy.isnan(A[i,:]))])

# Load some recorded marker location data.
plot_flying_data = 1

if plot_flying_data:
	Array = loadtxt('../../../usecases/collectMeasurements/test_data_pose_estimation.dat') 
	tCount = Array.shape[0]
	timeArray = Array[:,0]
	mArray = Array[:,2:]
	dt = numpy.median(numpy.diff(timeArray))
	print 'Based on logged data, camera operated at '+str(1/dt)+' Hz'
else:
	mArray = loadtxt('orocos1.dat') # Recorded with correct order???
	mArray = filter_out_rows_containing_nan(mArray)
	tCount = mArray.shape[0]
	timeArray = range(tCount)

poseArray=numpy.zeros((tCount,12))
euler_angleArray=numpy.zeros((tCount,3))
euler_angleArray2=numpy.zeros((tCount,3))
cov_rt_Array = numpy.zeros((tCount,12*12))
cov_rt_eigs_Array = numpy.zeros((tCount,12))
for timeIndex in xrange(mArray.shape[0]):
	m = mArray[timeIndex,:]

	pose_and_covrt_from_markers.setInput(m)
	pose_and_covrt_from_markers.evaluate()
	pose = pose_and_covrt_from_markers.output(0)
	cov_rt = pose_and_covrt_from_markers.output(1)
	pose = numpy.double(pose).squeeze()
	cov_rt = numpy.double(cov_rt).squeeze().reshape(12,12)

	#f_cov_rt.setInput(m)
	#f_cov_rt.evaluate()
	#cov_rt = f_cov_rt.output()
	#cov_rt = numpy.double(cov_rt)

	cov_rt_eigs = numpy.linalg.eigvals(cov_rt)

	poseArray[timeIndex,:] = pose
	cov_rt_Array[timeIndex,:] = cov_rt.reshape((12*12,))
	cov_rt_eigs_Array[timeIndex,:] = cov_rt_eigs

	R = pose[3:13]
	t = pose[0:3]
	R = R.reshape(3,3)

	r,p,y = roll_pitch_yaw_from_R(R)
	euler_angleArray[timeIndex,:] = numpy.array([r,p,y])

	r2,p2,y2 = get_rpy_from_pose(R,t) # rpy of plane relative to "neutral" flying position
	euler_angleArray2[timeIndex,:] = numpy.array([r2,p2,y2])

# Plot all of our variables as a function of "time"

# Position
pylab.close(1)
figure(1)
plotCount = 3;
subplot(plotCount,1,1)
plot(timeArray, poseArray[:,:3],'.')
ylabel('plane position in anchor frame [m]')
legend(['x','y','z'])

# Attitude
subplot(plotCount,1,2)
if 0:
	plot(timeArray, euler_angleArray[:,:3],'.')
	ylabel('Plane attitude in anchor frame [rad]')
	legend(['roll','pitch','yaw'])
if 1:
	plot(timeArray, euler_angleArray2[:,:3],'.')
	ylabel('Plane attitude relative to \n neutral "flying" orientation [rad]')
	legend(['Roll','Pitch','Yaw'])
if 0:
	plot(timeArray, poseArray[:,3:13],'.')
	ylabel('R: Plane attitude in anchor frame [unitless]')

# Covariance eigenvalues
subplot(plotCount,1,3)
plot(timeArray, cov_rt_eigs_Array,'.')
ylabel('eigenvalues [?]')

if plot_flying_data:
	xlabel('Time [s]')
else:
	xlabel('Sample index')

show()
