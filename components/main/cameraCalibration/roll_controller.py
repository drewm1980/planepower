#!/usr/bin/env python
import casadi
import numpy
from casadi import ssym
from rotation_matrices import roll_pitch_yaw_from_R

# Takes R,t that map points in body frame to anchor frame,
# Returns roll, pitch, and yaw relative to the "natural" flying position
def get_rpy_from_pose(R,t):
	if type(R)==casadi.SXMatrix:
		assert R.shape==(3,3)
		assert type(t)==casadi.SXMatrix
		assert t.shape==(3,1)
		R0 = casadi.SXMatrix(3,3)
	else:
		R0 = numpy.zeros((3,3))
	R0[0,2]=1
	R0[1,0]=1
	R0[2,1]=1
	if type(R)==casadi.SXMatrix:
		Rref = casadi.mul(R0.T,R)
	else:
		Rref = numpy.dot(R0.T,R)
	roll,pitch,yaw = roll_pitch_yaw_from_R(Rref)
	return roll, pitch, yaw

t1 = ssym('t1')
t2 = ssym('t2')
t3 = ssym('t3')
R11 = ssym('R11')
R12 = ssym('R12')
R13 = ssym('R13')
R21 = ssym('R21')
R22 = ssym('R22')
R23 = ssym('R23')
R31 = ssym('R31')
R32 = ssym('R32')
R33 = ssym('R33')
R = casadi.SXMatrix([[R11,R12,R13],[R21,R22,R23],[R31,R32,R33]])
t = casadi.vertcat([t1,t2,t3])

r,p,y = get_rpy_from_pose(R,t)

outputs = casadi.vertcat([r,p,y])
inputs = casadi.vertcat([t, casadi.vec(R.T)])
f = casadi.SXFunction([inputs],[outputs])
f.init()
f.generateCode('getRPYFromPose.c')
