#!/usr/bin/env python
import numpy
from numpy import zeros, hstack, dot
import casadi
from casadi import SXMatrix, ssym
from casadi_conveniences import sum_squares
from geometry import line_line_intersect
from numpy.linalg import norm
from rotation_matrices import invert_se3

# Take a point that is already in the camera frame, and project into the image.
def project(f,c,X):
	if casadi.SXMatrix in map(type,[f,c,X]):
		assert X.shape in [(3,1),(4,1)]
		assert f.shape==(2,1)
		assert c.shape==(2,1)
		if X.shape[0]==4:
			if X[3,0] != 1:
				X = X/X[3,0]
			X = X[:3,0]
		x = X[:2,0]*f + X[2,0]*c
		x = x/X[2,0]
		x = x[:2,0]
	else:
		if len(X)==4:
			X = X[:3]/X[3]
		assert len(X)==3
		assert len(f)==2
		assert len(c)==2
		x = X[:2]*f + X[2]*c
		x = x/X[2]
		x = x[:2]
	return x

# Take an image point and "unproject" to a point in the camera frame.
def unproject(f,c,x):
	if casadi.SXMatrix in map(type,[f,c,x]):
		assert x.shape==(2,1)
		assert f.shape==(2,1)
		assert c.shape==(2,1)
		# diag(f^-1) * (x-c) = X (up to scale)
		X = casadi.SXMatrix(3,1)
		X[:2,0] = f**-1 * (x - c)
		X[2,0] = 1.0
	else:
		# diag(f^-1) * (x-c) = X (up to scale)
		X = numpy.zeros((3,))
		X[:2] = f**-1 * (x - c)
		X[2] = 1.0
	return X

# Build casadi vectors f,c to represent the intrinsics for a single camera
def single_camera_intrinsics():
	fx=ssym('fx')
	fy=ssym('fy')
	cx=ssym('cx')
	cy=ssym('cy')
	f = SXMatrix(2,1)
	f[0,0] = fx
	f[1,0] = fy
	c = SXMatrix(2,1)
	c[0,0] = cx
	c[1,0] = cy
	return f,c

# Takes a stereo pinhole camera model and two points in the images,
# and estimate the 3d location of the point.
# Tinv is the inverse of T.  Includng Tinv as a parameter
# makes it easier to avoid duplicating subexpressions associated with it.
# Numpy implementation
def triangulateNumpy(f1, c1, f2, c2, T, Tinv, x1, x2):
	a1 = zeros((3,))
	a2 = unproject(f1,c1,x1)
	b1 = zeros((3,))
	b1 = hstack((b1,1.0))
	b1 = dot(T,b1)
	b1 = b1[:3]
	b2 = unproject(f2,c2,x2)
	b2 = hstack((b2,1.0))
	b2 = dot(T,b2)
	b2 = b2[:3]
	a3,b3,mua,mub = line_line_intersect(a1,a2,b1,b2)
	m3 = (a3+b3)*.5 # in camera 1 frame
	eSpace = norm(a3-b3) # distance between the lines, in distance units used by T (usually m)
	# Reproject m and compute approximate pixel error
	m1 = project(f1,c1,m3) # in camera 1 frame
	m2 = hstack((m3,1.0)) # in camera 1 frame
	m2 = dot(Tinv, m2) # in camera 2 frame
	m2 = project(f2,c2,m2)
	e1 = norm(m1 - x1)
	e2 = norm(m2 - x2)
	ePixel = (e1+e2)*.5
	return m3, eSpace, ePixel, m1, m2
# Casadi implementation
def triangulateCasadi(f1, c1, f2, c2, T, Tinv, x1, x2):
	a1 = SXMatrix(3,1)
	a2 = unproject(f1,c1,x1)
	b1 = SXMatrix(3,1)
	b1 = casadi.vertcat((b1,1.0))
	b1 = casadi.mul(T,b1)
	b1 = b1[:3,0]
	b2 = unproject(f2,c2,x2)
	b2 = casadi.vertcat((b2,1.0))
	b2 = casadi.mul(T,b2)
	b2 = b2[:3,0]
	a3,b3,mua,mub = line_line_intersect(a1,a2,b1,b2)
	m3 = (a3+b3)*.5 # in camera 1 frame
	eSpace = sum_squares(a3-b3) # distance between the lines, m^2
	# Reproject m and compute approximate pixel error
	m1 = project(f1,c1,m3) # in camera 1 frame
	m2 = casadi.vertcat((m3,1.0)) # in camera 1 frame
	m2 = casadi.mul(Tinv, m2) # in camera 2 frame
	m2 = project(f2,c2,m2)
	e1 = sum_squares(m1 - x1) # pixels^2
	e2 = sum_squares(m2 - x2) # pixels^2
	ePixel = (e1+e2)*.5
	return m3, eSpace, ePixel, m1, m2
def triangulate(f1, c1, f2, c2, T, Tinv, x1, x2):
	args = [f1, c1, f2, c2, T, Tinv, x1, x2]
	if casadi.SXMatrix in map(type,args):
		assert all([type(arg)==casadi.SXMatrix for arg in args])
		return triangulateCasadi(*args)
	else:
		return triangulateNumpy(*args)
