#!/usr/bin/env python
from time import time
import numpy
from numpy import pi
from numpy.random import rand
from numpy import arctan2
from numpy import sqrt

import casadi
from casadi import ssym, DMatrix, SXFunction, SXMatrix, msym
from casadi import MXFunction, msym
from casadi import NLP_X_INIT, NLP_X_OPT
from casadi import sin, cos

# Functions that return instances of some commonly used arrays in SE(3)
def Rx(theta):
	if type(theta)==SXMatrix:
		R = SXMatrix(4,4)
	R[0,0] = 1
	R[1,1] = 1
	R[2,2] = 1
	R[3,3] = 1
	c = cos(theta)
	s = sin(theta)
	R[1,1] = c
	R[2,1] = s
	R[1,2] = -s 
	R[2,2] = c
	return R
def Ry(theta):
	if type(theta)==SXMatrix:
		R = SXMatrix(4,4)
	R[0,0] = 1
	R[1,1] = 1
	R[2,2] = 1
	R[3,3] = 1
	c = cos(theta)
	s = sin(theta)
	R[0,0] = c
	R[0,2] = s
	R[2,0] = -s 
	R[2,2] = c
	return R
def Rz(theta):
	if type(theta)==SXMatrix:
		R = SXMatrix(4,4)
	R[0,0] = 1
	R[1,1] = 1
	R[2,2] = 1
	R[3,3] = 1
	c = cos(theta)
	s = sin(theta)
	R[0,0] = c
	R[1,0] = s
	R[0,1] = -s 
	R[1,1] = c
	return R
def Tx(delta):
	if type(delta)==SXMatrix:
		T = SXMatrix(4,4)
	T[0,0] = 1
	T[1,1] = 1
	T[2,2] = 1
	T[3,3] = 1
	T[0,3] = delta
	return T
def Ty(delta):
	if type(delta)==SXMatrix:
		T = SXMatrix(4,4)
	T[0,0] = 1
	T[1,1] = 1
	T[2,2] = 1
	T[3,3] = 1
	T[1,3] = delta
	return T
def Tz(delta):
	if type(delta)==SXMatrix:
		T = SXMatrix(4,4)
	T[0,0] = 1
	T[1,1] = 1
	T[2,2] = 1
	T[3,3] = 1
	T[2,3] = delta
	return T

# Take a skew symmetric matrix of the form:
# M = [ 0 -az ay
#       az 0 -ax
#       -ay ax 0 ]
# and return the vector [ax, ay, az].T
#
# Note: NO PROJECTION of any kind is performed.
# Use when you KNOW the matrix is actually skew symmetric
def unskew(M):
	ax = M[2,1]
	ay = M[0,2]
	az = M[1,0]
	return casadi.SXMatrix([ax,ay,az])

# Take a skew symmetric matrix of the form:
# M = [ 0 -az ay
#       az 0 -ax
#       -ay ax 0 ]
# and return the vector [ax, ay, az].T
#
# Note: This ~is the unskew operator defined in
#		Nonlinear MPC and MHE... [Gros, Zanon, Vukov, Diehl] draft.
#		Use on matrices that you suspect have drifted slightly 
#		from being skew symmetric.
def unskew_project(M):
	ax = onehalf * (M[2,1] - M[1,2])
	ay = onehalf * (M[0,2] - M[2,0])
	az = onehalf * (M[1,0] - M[0,1])
	return casadi.SXMatrix([ax,ay,az])

# Take a 4x4 matrix representation of a transformation in SE(3)
# and return the inverse. 
def invert_se3(T):
	R = T[:3,:3]
	t = T[:3,3]
	if type(T)==casadi.SXMatrix:
		Tinv = casadi.SXMatrix(4,4)
		Tinv[:3,3] = -casadi.mul(R.T,t)
	else:
		Tinv = numpy.zeros((4,4))
		Tinv[:3,3] = -numpy.dot(R.T,t)
	Tinv[:3,:3] = R.T
	Tinv[3,3] = 1
	return Tinv

# Take R (3x3) and t, and form T (4x4)
def pack_se3(R,t):
	casaditypes = [casadi.SXMatrix, casadi.DMatrix]
	if(type(R)==numpy.ndarray and type(t)==numpy.ndarray):
		T = numpy.zeros((4,4))
		T[:3,3]=t[:]
	elif(type(R) in casaditypes or type(t) in casaditypes):
		T = casadi.SXMatrix(4,4)
		T[:3,3]=t
	T[:3,:3]=R[:,:]
	T[3,3]=1
	return T

# Take T (3x3) and extract R (3x3) and t (3x1)
def unpack_se3(T):
	R = T[:3,:3]
	t = T[:3,3]
	return R,t

# Pack entries of T \in SE(3) into a vector
# consisting of the translation followed
# by R in row-major ordering.
def vectorize_se3(T):
	assert T.shape==(4,4)
	if type(T)==casadi.SXMatrix:
		return casadi.vertcat([T[:3,3],T[0,:3].T,T[1,:3].T,T[2,:3].T])
	else:
		return numpy.hstack([T[:3,3],T[0,:3],T[1,:3],T[2,:3]])

# Undo the packaging of vectorize_se3
def unvectorize_se3(pose):
	if type(pose)==casadi.SXMatrix:
		assert pose.shape==(12,1)
		T = casadi.SXMatrix(4,4)
		T[:3,3] = pose[0:3,0]
		T[0,:3] = pose[3:6,0].T
		T[1,:3] = pose[6:9,0].T
		T[2,:3] = pose[9:12,0].T
		T[3,3] = 1
	else:
		assert pose.shape==(12,)
		T = numpy.zeros((4,4))
		T[:3,3] = pose[:3]
		T[0,:3] = pose[3:6]
		T[1,:3] = pose[6:9]
		T[2,:3] = pose[9:12]
		T[3,3] = 1
	return T

def append_one(v):
	if(type(v)==numpy.ndarray):
		v2 = numpy.hstack((v,1))
	elif(type(v)==casadi.SXMatrix):
		v2 = casadi.vertcat([v,1])
	return v2

# Form a rotation matrix from euler angles.
# This Roll, Pitch, Yaw rotation parameterization was ported from KDL's Rotation::RPY
#  Also matches http://planning.cs.uiuc.edu/node102.html 
#   for the following mapping of variables:
#	  gamma ~ roll
#	   beta ~ pitch
#	   alpha ~ yaw
# ANGLES IN RADIANS
def R_from_roll_pitch_yaw(roll, pitch, yaw):
	ca1 = cos(yaw)
	sa1 = sin(yaw)
	cb1 = cos(pitch)
	sb1 = sin(pitch)
	cc1 = cos(roll)
	sc1 = sin(roll)
	ca1sb1 = ca1*sb1
	sa1sb1 = sa1*sb1
	if casadi.SXMatrix in map(type,[roll,pitch,yaw]):
		R = SXMatrix(3,3)
	else:
		R = numpy.zeros((3,3))
	R[0,0] = ca1*cb1
	R[0,1] = ca1sb1*sc1-sa1*cc1
	R[0,2] = ca1sb1*cc1+sa1*sc1
	R[1,0] = sa1*cb1
	R[1,1] = sa1sb1*sc1+ca1*cc1
	R[1,2] = sa1sb1*cc1-ca1*sc1
	R[2,0] = -sb1
	R[2,1] = cb1*sc1
	R[2,2] = cb1*cc1
	return R

## Take a (numeric) R, compute (numeric) roll, pitch, yaw
##  From http://planning.cs.uiuc.edu/node103.html 
#	NON-MONOTONIC MAPPING OF VARIABLES!!!
#	  gamma ~ roll
#	   beta ~ pitch
#	   alpha ~ yaw
#   This method assumes R[1,1]!=0 and R[2,2]!=0
def roll_pitch_yaw_from_R(R):
	alpha = arctan2(R[1,0],R[0,0]);
	beta =  arctan2(-R[2,0] , sqrt(R[2,1]**2+R[2,2]**2))
	gamma = arctan2(R[2,1],R[2,2])
	roll = gamma
	pitch = beta
	yaw = alpha
	return roll, pitch, yaw

# Invert any 3 angle rotation matrix parameterization
# using a combination of brute force initialization,
# and non-linear optimization.
# R is the (numeric) rotation matrix.
# parameterizationFunction is a function that generates
# a casadi symbolic matrix from 3 ssyms representing angles
# in the parameterization.
try:
    from casadi.tools import IpoptSolver
    def inverse_parameterization(R, parameterizationFunction):
        angle1 = ssym('angle1')
        angle2 = ssym('angle2')
        angle3 = ssym('angle3')
        Restimated = parameterizationFunction(angle1,angle2,angle3)
        E = R - Restimated;
        e = casadi.sumAll(E*E)
        f = SXFunction([casadi.vertcat([angle1,angle2,angle3])], [e])
        f.init()
        anglerange = numpy.linspace(-1*numpy.pi,1*numpy.pi,25)
        best_eTemp = 9999999999999
        best_angle1 = numpy.nan
        best_angle2 = numpy.nan
        best_angle3 = numpy.nan
        for i in xrange(len(anglerange)):
            for j in xrange(len(anglerange)):
                for k in xrange(len(anglerange)):
                    r = anglerange[i]
                    p = anglerange[j]
                    y = anglerange[k]
                    f.setInput([r,p,y])
                    f.evaluate()
                    eTemp = f.output(0)
                    if eTemp<best_eTemp:
                        best_eTemp = eTemp
                        best_angle1 = r
                        best_angle2 = p
                        best_angle3 = y
        if 0:
            V = msym("V",(3,1))
            fval = f.call([V])
            fmx = MXFunction([V],fval)
            nlp_solver = IpoptSolver(fmx)
        else:
            nlp_solver = IpoptSolver(f)

        nlp_solver.setOption('generate_hessian',True)
        nlp_solver.setOption('expand_f',True)
        nlp_solver.setOption('expand_g',True)
        nlp_solver.init()
        nlp_solver.setInput([best_angle1,best_angle2,best_angle3],NLP_X_INIT)
        nlp_solver.solve()
        sol =  nlp_solver.output(NLP_X_OPT).toArray()
        sol =  nlp_solver.output(NLP_X_OPT).toArray()

        return sol
except ImportError:
    print "Warning: IpoptSolver not available. inverse_parameterization won't be defined."

if 0:
	rpy1 = rand(3,1)
	R1 = R_from_roll_pitch_yaw_numeric(rpy1[0],rpy1[1],rpy1[2])

	t1 = time()
	rpy2 = inverse_parameterization(R1, R_from_roll_pitch_yaw_symbolic)
	t2 = time()
	t = t2 - t1
	R2 = R_from_roll_pitch_yaw_numeric(rpy2[0],rpy2[1],rpy2[2])

	rpy3 = roll_pitch_yaw_from_R(R1)
	R3 = R_from_roll_pitch_yaw_numeric(rpy3[0],rpy3[1],rpy3[2])

def test_R_from_roll_pitch_yaw():
	# Use it twice for different sets of angles
	r1= ssym('roll')
	p1= ssym('pitch')
	y1= ssym('yaw')
	r2= ssym('roll')
	p2= ssym('pitch')
	y2= ssym('yaw')

	R1 = R_from_roll_pitch_yaw_symbolic(r1, p1, y1)
	R2 = R_from_roll_pitch_yaw_symbolic(r2, p2, y2)

	# Still building up everything symbolic as Matrix<SX>
	R = R1*R2
	#from casadi.tools.graph import dotdraw
	#dotdraw(R)

	# As one of the last steps, convert to SXFunction for code generation, optimization, etc...
	v = SXMatrix(6,1)
	v[0,0] = r1
	v[1,0] = p1
	v[2,0] = y1
	v[3,0] = r2
	v[4,0] = p2
	v[5,0] = y2
	f_RPY = SXFunction([v],[R])
	f_RPY.init()

