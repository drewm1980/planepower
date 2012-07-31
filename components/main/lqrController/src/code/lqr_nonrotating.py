#!/usr/bin/env python
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
#from kinetics.flatkinematics import skew
from scipy.linalg.matfuncs import sqrtm


from scipy.linalg.matfuncs import toreal

K = NP.loadtxt("K.dat")
K = DMatrix(K) #scaling of LQR gains for the new control derivatives
u0 = NP.loadtxt("u0.dat")
xr = NP.loadtxt("x_ref.dat")

X = ssym("X",22,1)
X_ref = ssym("X_ref",22,1)

X_reduced =  X[0:18]
X_ref_reduced =  X_ref[0:18]
dX = X_reduced-X_ref_reduced
# We had to put X_ref[3] and X_ref[4] to zero (4 and 5 in matlab..)
#dX[3] = X_rotated[3]
#dX[4] = X_rotated[4]
u = -mul(K,dX)+u0
#The control input ordereing does not match Mario's model
u_transformed = ssym("u",2,1)
u_transformed[0] = u[0]
u_transformed[1] = u[1]
#u_transformed[2] = u[2]
#u_transformed[3] = u[3]

f = SXFunction([X,X_ref],[u_transformed])
f.init()
#
f.generateCode("lqr_nonrotating.c")
