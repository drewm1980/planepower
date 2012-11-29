import numpy
from numpy import loadtxt, vstack, hstack, isnan, dot, zeros, dot, nan, median
import pylab
from pylab import plot, hold

# Load some points I got by:
	# 1. Running LEDTracker and waving around the plane
	# 2. Parsing data of the orocos.log file
p1 = loadtxt('orocos1.dat')
# Marker locations for right camera come first (I double-checked)
rp1 = p1[:,:6] # columns are ru rv gu gv bu bv or something (I may have the color order wrong).
lp1  = p1[:,6:]
# Flatten the data; as long as we have correspondence, color doesn't matter.
rp2 = vstack((rp1[:,0:2], rp1[:,2:4], rp1[:,4:6]))
lp2 = vstack((lp1[:,0:2], lp1[:,2:4], lp1[:,4:6]))
p2 = hstack((rp2, lp2))
# Filter out nan measurements
l3 = [p2[i,:] for i in xrange(p2.shape[0]) if not any(isnan(p2[i,:]))]
point_correspondences = vstack(l3)

