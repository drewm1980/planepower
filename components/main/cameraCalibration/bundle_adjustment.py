#!/usr/bin/env python
import casadi
import numpy
from time import time
from numpy import loadtxt, vstack, hstack, isnan, dot, zeros, dot, nan, median
from numpy.random import rand 
from numpy.linalg import norm
from casadi import ssym, DMatrix, SXFunction, SXMatrix, mul
from casadi.tools import IpoptSolver
from casadi import NLP_X_INIT, NLP_X_OPT, NLP_LBG, NLP_UBG
import pylab
from pylab import plot, hold, show
from rotation_matrices import invert_se3, pack_se3, unpack_se3
from rotation_matrices import R_from_roll_pitch_yaw, roll_pitch_yaw_from_R
from camera_model import project, unproject, triangulate
from casadi_conveniences import flatten_sx

from load_calibration_data import f1,c1,f2,c2,T
from load_calibration_sequences import point_correspondences;

import camera_model
from camera_model import triangulate, single_camera_intrinsics

from load_calibration_sequences import point_correspondences as point_correspondences_numpy;
point_correspondences = casadi.SXMatrix(point_correspondences_numpy)

# For first attempt, only perform bundle adjustment for extrinsics.
from load_calibration_data_casadi import T as T_initial
from load_calibration_data_casadi import Tinv as Tinv_initial
from load_calibration_data_casadi import f1
from load_calibration_data_casadi import c1
from load_calibration_data_casadi import f2
from load_calibration_data_casadi import c2

# The extrinsics will be free parameters
roll = ssym('roll')
pitch = ssym('pitch')
yaw = ssym('yaw')
tx = ssym('tx')
ty = ssym('ty')
tz = ssym('tz')
t = casadi.vertcat([tx,ty,tz])

# Build expression for the camera pair extrinsics in terms of roll, pitch, and yaw
R = R_from_roll_pitch_yaw(roll, pitch, yaw)
T = pack_se3(R,t)
Tinv = invert_se3(T)

# Compute initial values for roll, pitch, yaw
R_initial,t_initial = unpack_se3(T_initial)
roll_initial,yaw_initial,pitch_initial = roll_pitch_yaw_from_R(R_initial)
inputs_initial = [roll_initial, pitch_initial, yaw_initial
					, t_initial[0], t_initial[1], t_initial[2]]
inputs_initial = flatten_sx(inputs_initial)

print "Constructing objective..."
objective = SXMatrix(1,1)
for i in xrange(point_correspondences.shape[0]):
	x1 = point_correspondences[i,:2].T
	x2 = point_correspondences[i,2:].T
	m3, eSpace, ePixel, m1, m2 = triangulate(f1, c1, f2, c2, T, Tinv, x1, x2)
	objective = objective + eSpace  # units of m^2

print "Constructing constraint..."
temp = t_initial[0]*t_initial[0] + t_initial[1]*t_initial[1] + t_initial[2]*t_initial[2]  
temp = flatten_sx(temp)
constraint = t[0]*t[0] + t[1]*t[1] + t[2]*t[2] - temp

print "Objective has "+str(casadi.countNodes(objective))+" nodes"

print "Constructing SXFunction..."
inputs = casadi.vertcat((roll,pitch,yaw,tx,ty,tz))
outputs = objective
f = SXFunction([inputs],[outputs])
f.init()
f_constraint = SXFunction([inputs],[constraint])
f_constraint.init()

print "Constructing IpoptSolver..."
nlp_solver = IpoptSolver(f, f_constraint)
nlp_solver.setOption('generate_hessian',True)
nlp_solver.init()

nlp_solver.setInput(inputs_initial,NLP_X_INIT)
nlp_solver.setInput(0,NLP_LBG)
nlp_solver.setInput(0,NLP_UBG)

t0 = time()
nlp_solver.solve()
t0 = time()

sol =  nlp_solver.output(NLP_X_OPT).toArray()

print inputs_initial
print sol.T
solR = R_from_roll_pitch_yaw(sol[0],sol[1],sol[2])
solt = sol[3:].squeeze()
solT = pack_se3(solR,solt)

numpy.savetxt('T2.dat', solT)

#sol =  DMatrix(nlp_solver.output(NLP_X_OPT))


# For second attempt, try freeing the camera intrinsics
# This might have a degeneracy where the estimated marker locations
# get squished by pushing focal lengths to zero in one direction.
#f1,c1 = single_camera_intrinsics()
#f2,c2 = single_camera_intrinsics()


