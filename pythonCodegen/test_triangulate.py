#!/usr/bin/env python
import casadi
import numpy
from numpy import loadtxt, vstack, hstack, isnan, dot, zeros, dot, nan, median
from numpy.random import rand 
from numpy.linalg import norm
from casadi import ssym, DMatrix, SXFunction, SXMatrix, mul
import pylab
from pylab import plot, hold, show
from rotation_matrices import invert_se3
from camera_model import project, unproject, triangulate

from load_calibration_data import f1,c1,f2,c2,T
from load_calibration_sequences import point_correspondences;

if 0:
	# Load T from after bundle adjustment
	T = numpy.loadtxt('T2.dat')

# Display pixel errors
es = numpy.zeros((point_correspondences.shape[0]))
ep = numpy.zeros((point_correspondences.shape[0]))
for i in xrange(len(point_correspondences)):
	x1 = point_correspondences[i,:2]
	x2 = point_correspondences[i,2:]
	Tinv = invert_se3(T)
	m,eSpace,ePixel,m1,m2 = triangulate(f1,c1,f2,c2,T,Tinv,x1,x2)
	es[i] = eSpace
	ep[i] = ePixel
	plot([x1[0],m1[0]],[x1[1],m1[1]],'k-', x1[0],x1[1],'g.', m1[0],m1[1],'r.')
	hold('on')
# Display image bounding box
plot([0,1600,1600,0,0],[0,0,1200,1200,0],'k-')
pylab.axis([0, 1600, 0, 1200])
pylab.show()

mes = median(es) 
mep = median(ep)
print "Median spatial error is about "+str(mes)+' meters'
print "Median reprojection error is about "+str(mep)+' pixels'
