#!/usr/bin/env python
from time import time, sleep
import numpy
import os
import realtime

from ctypes import cdll, c_int, POINTER, c_float, byref
lib=cdll.LoadLibrary('./bitbang_spi.so')
f = lib.bitbang_init
f()
f = lib.read_angle_sensors
f.argtypes=[POINTER(c_float),POINTER(c_float)]

def read_angle_sensors():
	"""Returns the line angles (azimuth,elevation) in radians"""
	a = c_float()
	b = c_float()
	f(byref(a),byref(b))
	return a.value,b.value

if __name__=="__main__":
	realtime.enable()
	str = "Avg. time in sec: "
	t0 = time()
	for i in range(1,1001):
		#realtime.sleep(.001)
		#t1 = time()-t0
		a,b = read_angle_sensors()
		#t2 = time()-t0
		#t3 = t2 - t1
		#print a,b,str,t3
		#print t1,t2,a,b,t3
	t3 = (time() - t0)/1000
	print str, t3
	realtime.disable()


