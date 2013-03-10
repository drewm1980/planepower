#!/usr/bin/env python
import numpy
from numpy import cross, dot
import casadi

# Takes a n x 1 casadi matrix and returns a normalized version of it.
def normalize_casadi(x):
	n = casadi.sumAll(x*x)
	n = casadi.sqrt(n)
	nx = x/n
	return nx

# Casadi implementation of vector cross product
def cross_casadi(a,b):
	assert a.shape==(3,1)
	assert b.shape==(3,1)
	a = a.toArray().squeeze()
	b = b.toArray().squeeze()
	c = numpy.cross(a,b)
	return casadi.SXMatrix(c)

# Compute the rigid body motion that transforms three points a1, a2, a3 into the three points b1, b2, b3, in the least-squared sense.
# bi = R*ai + t up to noise.
# ai, bi are 3D vectors stored as 3x1 casadi Matrix<SX>.
# This is a closed form solution that requires no trig; should be very fast.
#
# R, t = argmin( ||(b1 - (R*a1 + t)) ||^2 + ||(b2 - (R*a2 + t)) ||^2 + ||(b3 - (R*a3 + t)) ||^2 )
# 	where shorthand is: A = [a1 a2 a3], B = [b1 b2 b3]
#
# I have a hunch there is an even simpler way of computing this; my geometric reasoning often results
# in long code.
def register_three_points_casadi(a1,a2,a3, b1,b2,b3):
	assert a1.shape==(3,1)
	assert a2.shape==(3,1)
	assert a3.shape==(3,1)
	assert b1.shape==(3,1)
	assert b2.shape==(3,1)
	assert b3.shape==(3,1)
	A = casadi.horzcat((a1,a2,a3)) # place a1,a2,a3 into columns of A
	B = casadi.horzcat((b1,b2,b3)) # place b1,b2,b3 into columns of B

	temp1a = normalize_casadi(a2 - a1)
	temp2a = normalize_casadi(a3 - a1)
	temp1b = normalize_casadi(b2 - b1)
	temp2b = normalize_casadi(b3 - b1)

	na = normalize_casadi(cross_casadi(temp1a, temp2a))
	nb = normalize_casadi(cross_casadi(temp1b, temp2b))

	# [temp1a na temp3a] is a rotation matrix.
	temp3a = cross_casadi(temp1a, na)
	temp3b = cross_casadi(temp1b, nb)

	# Find the rotation that aligns the normals with the z-axis.
	# Be careful to preserve handedness.
	Ra = casadi.horzcat((temp3a, temp1a, na)).T
	Rb = casadi.horzcat((temp3b, temp1b, nb)).T
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
	Rxy = casadi.SXMatrix(3,3)
	Rxy[0,0] = g
	Rxy[0,1] = -h
	Rxy[1,0] = h
	Rxy[1,1] = g
	Rxy[2,2] = 1

	# Combine the transformations to the the global R, t we're looking for.
	#R = Rb.T * Rxy * Ra
	mul = casadi.mul
	R = mul(mul(Rb.T, Rxy), Ra)
	#t = -Rb.T * Rxy * mua + Rb.T * mub
	t = mul(-Rb.T, mul(Rxy, mua)) + mul(Rb.T, mub)

	return R,t

