#!/usr/bin/env python
import casadi
from load_calibration_data import *
from rotation_matrices import invert_se3

# Camera 1 intrinsics
f1 = casadi.SXMatrix(f1)
c1 = casadi.SXMatrix(c1)
# Camera 2 intrinsics
f2 = casadi.SXMatrix(f2)
c2 = casadi.SXMatrix(c2)
# Extrinsics
RPC1 = casadi.SXMatrix(RPC1) # maps points in camera 1 frame into arm frame
RPC2 = casadi.SXMatrix(RPC2) # maps points in camera 2 frame into arm frame
T = casadi.SXMatrix(T) # maps points in camera 2 frame into camera 1 frame
RPC1inv = casadi.SXMatrix(RPC1inv) # maps points arm frame into camera 1 frame
RPC2inv = casadi.SXMatrix(RPC2inv) # maps points arm frame into camera 2 frame 
Tinv = casadi.SXMatrix(Tinv) # maps points in camera 1 frame into camera 2 frame
# Markers in body (attached to plane) frame
m1_body = casadi.SXMatrix(m1_body)
m2_body = casadi.SXMatrix(m2_body)
m3_body = casadi.SXMatrix(m3_body)
