#!/usr/bin/env python
import numpy
from numpy import loadtxt, dot
from rotation_matrices import invert_se3

cameraspecs='../properties/cameras'
markerspecs='../properties/markers'

# Intrinsics for camera 1
PdatC1 = loadtxt(cameraspecs+'/PC1.dat') # fx fy cx cy
f1 = PdatC1[:2]
c1 = PdatC1[2:]

# Intrinsics for camera 2
PdatC2 = loadtxt(cameraspecs+'/PC2.dat') # fx fy cx cy
f2 = PdatC2[:2]
c2 = PdatC2[2:]

# Extrinsics
RPC1 = loadtxt(cameraspecs+'/RPC1.dat') # maps points in camera 1 frame into points in arm frame
RPC2 = loadtxt(cameraspecs+'/RPC2.dat') # maps points in camera 2 frame into points in arm frame
T = dot(invert_se3(RPC1),RPC2)  # Maps points in camera 2 frame into camera 1 frame

RPC1inv = invert_se3(RPC1) # maps points arm frame into camera 1 frame
RPC2inv = invert_se3(RPC2) # maps points arm frame into camera 2 frame 
Tinv = invert_se3(T) # Maps points in camera 1 frame into camera 2 frame

# Marker locations in the body (attached to plane) frame
m1_body = loadtxt(markerspecs+'/pos_marker_body1.dat')
m2_body = loadtxt(markerspecs+'/pos_marker_body2.dat')
m3_body = loadtxt(markerspecs+'/pos_marker_body3.dat')

