#!/usr/bin/env python
import numpy
from numpy import cross, dot
from geometry import line_line_intersect

# Takes a 1d numpy vector and returns a normalized version of it.
def normalize_numeric(x):
	nx = x.copy()
	n = x[0]*x[0] + x[1]*x[1] + x[2]*x[2]
	n = numpy.sqrt(n)
	nx /= n
	return nx

# Compute the rigid body motion that transforms three points a1, a2, a3 into the three points b1, b2, b3, in the least-squared sense.
# bi = R*ai + t up to noise.
# ai, bi are 3D vectors.
# This is a closed form solution that requires no trig; should be very fast.
#
# R, t = argmin( ||(b1 - (R*a1 + t)) ||^2 + ||(b2 - (R*a2 + t)) ||^2 + ||(b3 - (R*a3 + t)) ||^2 )
# 	where shorthand is: A = [a1 a2 a3], B = [b1 b2 b3]
#
# I have a hunch there is an even simpler way of computing this; my geometric reasoning often results
# in long code.
def register_three_points(a1,a2,a3, b1,b2,b3):
	assert len(a1)==3
	assert len(a2)==3
	assert len(a3)==3
	assert len(b1)==3
	assert len(b2)==3
	assert len(b3)==3
	#A = numpy.zeros((3,3))
	#B = numpy.zeros((3,3))
	#A[:,0] = a1
	#A[:,1] = a2
	#A[:,2] = a3
	#B[:,0] = b1
	#B[:,1] = b2
	#B[:,2] = b3
	A = numpy.vstack((a1,a2,a3)).T # place a1,a2,a3 into columns of A
	B = numpy.vstack((b1,b2,b3)).T # place b1,b2,b3 into columns of B

	temp1a = normalize_numeric(a2 - a1)
	temp2a = normalize_numeric(a3 - a1)
	temp1b = normalize_numeric(b2 - b1)
	temp2b = normalize_numeric(b3 - b1)

	na = normalize_numeric(cross(temp1a, temp2a))
	nb = normalize_numeric(cross(temp1b, temp2b))

	# [temp1a na temp3a] is a rotation matrix.
	temp3a = cross(temp1a, na)
	temp3b = cross(temp1b, nb)

	# Find the rotation that aligns the normals with the z-axis.
	# Be careful to preserve handedness.
	Ra = numpy.vstack((temp3a, temp1a, na))
	Rb = numpy.vstack((temp3b, temp1b, nb))
	Atilde = dot(Ra,A) # 3x3
	Btilde = dot(Rb,B)

	# Subtract off the means of the points.
	mua = Atilde.mean(axis=1) # means of the rows
	mub = Btilde.mean(axis=1) # means of the rows
	AHat = Atilde - mua.reshape((3,1)) # make mua a column, broadcast for subtraction
	BHat = Btilde - mub.reshape((3,1)) # make mub a column, broadcast for subtraction

	# Compute the x-y plane rotation that aligns the points in least squared sense.
	aRow1 = AHat[0,:]
	aRow2 = AHat[1,:]
	bRow1 = BHat[0,:]
	bRow2 = BHat[1,:]
	g = (aRow1*bRow1 + aRow2*bRow2).sum()
	h = (aRow1*bRow2 - aRow2*bRow1).sum()
	temp = numpy.sqrt(g*g + h*h)
	g = g/temp  # now, g = cos(theta) where theta is the optimal rotation amount.
	h = h/temp  # similarly, h = sin(theta)

	# Go ahead and format as a full 3D rotation matrix. 
	#Rxy = [g -h 0; h g 0; 0 0 1];  
	Rxy = numpy.zeros((3,3))
	Rxy[0,0] = g
	Rxy[0,1] = -h
	Rxy[1,0] = h
	Rxy[1,1] = g
	Rxy[2,2] = 1

	# Combine the transformations to the the global R, t we're looking for.
	#R = Rb.T * Rxy * Ra
	R = dot(dot(Rb.T, Rxy), Ra)
	#t = -Rb.T * Rxy * mua + Rb.T * mub
	t = dot(-Rb.T, dot(Rxy, mua)) + dot(Rb.T, mub)

	return R,t

