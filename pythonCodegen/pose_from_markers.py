#!/usr/bin/env python
import numpy
from casadi import ssym, mul
import casadi
from rotation_matrices import pack_se3, vectorize_se3

from camera_model import triangulate
from register_three_points_casadi import register_three_points_casadi

from load_calibration_data_casadi import f1,c1,f2,c2,RPC1,RPC2,T,Tinv,m1_body,m2_body,m3_body

from simple_codegen import generateSimpleCode

c1m1u = ssym('c1m1u') # Marker 1, camera 1, u image coordinate
c1m1v = ssym('c1m1v')
c1m2u = ssym('c1m2u')
c1m2v = ssym('c1m2v')
c1m3u = ssym('c1m3u')
c1m3v = ssym('c1m3v')
c2m1u = ssym('c2m1u')
c2m1v = ssym('c2m1v')
c2m2u = ssym('c2m2u')
c2m2v = ssym('c2m2v')
c2m3u = ssym('c2m3u')
c2m3v = ssym('c2m3v')

markers = [c1m1u, c1m1v, c1m2u, c1m2v, c1m3u, c1m3v,
			c2m1u, c2m1v, c2m2u, c2m2v, c2m3u, c2m3v]
markers = casadi.vertcat(markers)

c1m1 = casadi.vertcat([c1m1u,c1m1v])
c2m1 = casadi.vertcat([c2m1u,c2m1v])
c1m2 = casadi.vertcat([c1m2u,c1m2v])
c2m2 = casadi.vertcat([c2m2u,c2m2v])
c1m3 = casadi.vertcat([c1m3u,c1m3v])
c2m3 = casadi.vertcat([c2m3u,c2m3v])

mlist = []
for i in xrange(3):
	x1 = [c1m1, c1m2, c1m3][i]
	x2 = [c2m1, c2m2, c2m3][i]
	m, m_eSpace, m_ePixel, ign1, ign2 = triangulate(f1,c1,f2,c2,T,Tinv,x1,x2)
	mlist.append(m)

# Casadi expressions for our three markers in R^3 in camera 1 frame
m1_cam1 = mlist[0]
m2_cam1 = mlist[1]
m3_cam1 = mlist[2]

R,t = register_three_points_casadi( m1_body, m2_body, m3_body, m1_cam1, m2_cam1, m3_cam1)

T_body_to_cam1 = pack_se3(R,t) 

T_body_to_anchor = casadi.mul(RPC1,T_body_to_cam1)
pose = vectorize_se3(T_body_to_anchor)
pose_from_markers = casadi.SXFunction([markers], [pose])
pose_from_markers.init()

J = pose_from_markers.jac() # A 12 x 12 SX

sigma = 10 # pixels
cov_uv = casadi.SXMatrix.eye(12)
cov_uv[:] = sigma*sigma # pixels^2
cov_rt = mul(mul(J,cov_uv),J.T)

# Symbolic matrix inverse based on QR without pivoting
def inverse_symbolic(x):
	assert type(x)==casadi.SXMatrix
	xinv = casadi.solve(x, casadi.SXMatrix.eye(x.size1()))
	return xinv

# Take a casadi Matrix<SX>, and vectorize in row major order.
def vectorize_row_major_casadi(M):
	assert type(M)==casadi.SXMatrix
	return casadi.vec(M.T)

#print "About to invert cov_rt in casadi"
#invcov_rt = casadi.inv(cov_rt)  # Crashes casadi; too big.
invcov_rt = inverse_symbolic(cov_rt) # SX

pose_and_covrt_from_markers = casadi.SXFunction([markers],[pose, vectorize_row_major_casadi(cov_rt)])
pose_and_covrt_from_markers.init()

pose_and_J_from_markers = casadi.SXFunction([markers],[pose, vectorize_row_major_casadi(J)])
pose_and_J_from_markers.init()

pose_and_covrtinv_from_markers = casadi.SXFunction([markers],[pose, vectorize_row_major_casadi(invcov_rt)])
pose_and_covrtinv_from_markers.init()

pose_and_covrt_and_covrtinv_from_markers = casadi.SXFunction([markers],[pose,
																	vectorize_row_major_casadi(cov_rt),
																	vectorize_row_major_casadi(invcov_rt)])
pose_and_covrt_and_covrtinv_from_markers.init()

# Last Step: Code Generation!

generateSimpleCode(pose_and_covrt_and_covrtinv_from_markers,
                   'pose_from_markers.h',
                  docstring="""// Estimate camera pose (and covariance estimate) purely from a set of marker positions.  See pose_from_markers.py""")

