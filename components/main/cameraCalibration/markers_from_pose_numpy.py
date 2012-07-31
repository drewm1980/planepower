import numpy
import casadi
from casadi import ssym, mul, SXMatrix

from rotation_matrices import pack_se3, vectorize_se3, unvectorize_se3, append_one
from camera_model import triangulate, project
from register_three_points import register_three_points
from load_calibration_data import *

def markers_from_pose(pose):
	T_body_to_anchor = unvectorize_se3(pose)
	markers_body = [m1_body, m2_body, m3_body]
	markers_body = map(append_one, markers_body)
	markers = numpy.zeros((12)) 
	RPCinvs = [RPC1inv, RPC2inv] # These map points in arm frames to camera frames
	fs = [f1,f2]
	cs = [c1,c2]
	for ci in xrange(2):
		for mi in xrange(3):
			m_anchor = numpy.dot(T_body_to_anchor, markers_body[mi])
			m_cam = numpy.dot(RPCinvs[ci],m_anchor)
			uv = project(fs[ci],cs[ci],m_cam)
			starti = ci*6+mi*2
			markers[starti:starti+2] = uv
	return markers

pose = vectorize_se3(numpy.eye(4))
markers = markers_from_pose(pose)


