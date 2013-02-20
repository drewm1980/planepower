# -*- coding: utf-8 -*-
"""
Created on Thu Nov 24 15:59:28 2011

@author: kurt
"""
from casadi import *
import numpy as NP
import matplotlib.pyplot as plt
import time
from casadi.tools import *
from kinetics.flatkinematics import skew
from scipy.linalg.matfuncs import sqrtm
from scipy.linalg.matfuncs import toreal

K = NP.loadtxt("K.dat")
K = DMatrix(K)
u0 = NP.loadtxt("u0.dat")
xr = NP.loadtxt("x_ref.dat")

X = ssym("X",22,1)
X_ref = ssym("X_ref",22,1)
#Rotation over delta:
X_rotated = ssym("X_rotated",22,1)
X_rotated.setAll(0.0)
X_rotated[0] = X[0]*cos(X[20]) + X[1]*sin(X[20])
X_rotated[1] = X[1]*cos(X[20]) - X[0]*sin(X[20])
X_rotated[2] = X[2]
X_rotated[3] = X[3]*cos(X[20]) + X[4]*sin(X[20]) + X[1]*X[21]*cos(X[20]) - X[0]*X[21]*sin(X[20])
X_rotated[4] = X[4]*cos(X[20]) - X[3]*sin(X[20]) - X[0]*X[21]*cos(X[20]) - X[1]*X[21]*sin(X[20])
X_rotated[5] = X[5]
X_rotated[6] = X[6]*cos(X[20]) + X[9]*sin(X[20])
X_rotated[7] = X[7]*cos(X[20]) + X[10]*sin(X[20])
X_rotated[8] = X[8]*cos(X[20]) + X[11]*sin(X[20])
X_rotated[9] = X[9]*cos(X[20]) - X[6]*sin(X[20])
X_rotated[10] = X[10]*cos(X[20]) - X[7]*sin(X[20])
X_rotated[11] = X[11]*cos(X[20]) - X[8]*sin(X[20])
X_rotated[12] = X[12]
X_rotated[13] = X[13]
X_rotated[14] = X[14]
X_rotated[15] = X[15]
X_rotated[16] = X[16]
X_rotated[17] = X[17]
X_rotated[18] = X[18]
X_rotated[19] = X[19]
X_rotated[20] = 0.0
X_rotated[21] = X[21]

dX = X_rotated-X_ref
# We had to put X_ref[3] and X_ref[4] to zero (4 and 5 in matlab..)
dX[3] = X_rotated[3]
dX[4] = X_rotated[4]
u = -mul(K,dX)+u0
#The control input ordereing does not match Mario's model
u_transformed = ssym("u",4,1)
u_transformed[0] = u[1]
u_transformed[1] = u[0]
u_transformed[2] = u[2]
u_transformed[3] = u[3]
f = SXFunction([X,X_ref],[u_transformed])
f.init()

f.generateCode("lqr.cpp")