import numpy
import casadi
from pose_from_markers import pose_from_markers
from markers_from_pose import f_markers_from_pose
from test_pose_from_markers import mArray

# Make sure our camera measurement and reconstruction
# functions are inverses of each-other.
#for timeIndex in xrange(mArray.shape[0]):
for timeIndex in [0]:
	m = mArray[timeIndex,:]

	pose_from_markers.setInput(m)
	pose_from_markers.evaluate()
	pose = pose_from_markers.output()
	pose = numpy.double(pose).squeeze()

	f_markers_from_pose.setInput(pose)
	f_markers_from_pose.evaluate()
	markers2 = f_markers_from_pose.output()
	markers2 = numpy.double(markers2).squeeze()
	# Due to noise, we expect markers2 to differ from m, but not by much.
	if(any(abs(m-markers2)>12)):
		print m
		print markers2

	pose_from_markers.setInput(markers2)
	pose_from_markers.evaluate()
	pose2 = pose_from_markers.output()
	pose2 = numpy.double(pose2).squeeze()
	# pose2 should now match pose up to floating point precision.
	if(any(abs(pose-pose2)>.001)):
		print "TEST FAILED!!"
		print pose
		print pose2
	else:
		print "TEST PASSED"


