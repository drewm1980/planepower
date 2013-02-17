#!/usr/bin/env python
import numpy
import casadi
from casadi import ssym, mul, SXMatrix

from rotation_matrices import pack_se3, vectorize_se3, unvectorize_se3, append_one
from camera_model import triangulate, project
from register_three_points_casadi import register_three_points_casadi
from load_calibration_data_casadi import *

from simple_codegen import generateSimpleCode

# Vector of uninitialized casadi variables for the pose input
pose = ssym('pose',12,1)

# Return a casadi expression that computes the markers from the pose.
def markers_from_pose_casadi(pose):
	T_body_to_anchor = unvectorize_se3(pose)
	markers_body = [m1_body, m2_body, m3_body]
	markers_body = map(append_one, markers_body)
	markers = SXMatrix.zeros(12,1) 
	RPCinvs = [RPC1inv, RPC2inv] # These map points in arm frames to camera frames
	fs = [f1,f2]
	cs = [c1,c2]
	for ci in xrange(2):
		for mi in xrange(3):
			m_anchor = mul(T_body_to_anchor, markers_body[mi])
			m_cam = mul(RPCinvs[ci],m_anchor)
			uv = project(fs[ci],cs[ci],m_cam)
			starti = ci*6+mi*2
			markers[starti:starti+2,0] = uv
	return markers

#from casadi.tools import dotdraw
#dotdraw(markers)

markers = markers_from_pose_casadi(pose)
f_markers_from_pose = casadi.SXFunction([pose],[markers])
f_markers_from_pose.init()
#f_markers_from_pose.generateCode('markers_from_pose.c')
generateSimpleCode(f_markers_from_pose,'markers_from_pose.h',
                  """// Compute the camera markers positions from camera pose. See markers_from_pose.py""")

f_markers_from_pose.setInput(vectorize_se3(numpy.eye(4)))
f_markers_from_pose.evaluate()
markers1 = f_markers_from_pose.output(0)
markers1 = numpy.double(markers1).squeeze()
