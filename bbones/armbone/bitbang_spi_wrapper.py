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
a = c_float()
b = c_float()
pa = byref(a)
pb = byref(b)

def read_angle_sensors():
	f(pa,pb)
	return a.value,b.value

def test():
	sleeptimes = numpy.linspace(11,11,256) # ms
	trials = len(sleeptimes)
	print "Will do " + str(trials) + " trials"
	actual_sleeptimes = numpy.ones(trials)*numpy.nan
	i = 0
	while i<trials:
		sleep(.01) # just to spread out the sleeps that we are actually timing
		t0 = time()
		#realtime.busy_sleep(sleeptimes[i]*.001)
		sleep(sleeptimes[i]*.001)
		t00 = time()
		actual_sleeptimes[i] = (t00-t0) * 1000.0 # ms
		i += 1

	wakeup_times = actual_sleeptimes - sleeptimes
	best = min(wakeup_times)
	worst = max(wakeup_times)
	median = numpy.median(wakeup_times)
	print "Wakeup time (ms) Best: " + str(best) + " Median: " + str(median) + " Worst: " + str(worst)

#realtime.enable()
#test()
read_angle_sensors()
#realtime.disable()


