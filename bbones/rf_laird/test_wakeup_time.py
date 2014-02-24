#!/usr/bin/env python

import random
import string
from time import time, sleep
import numpy
import os

import Adafruit_BBIO.GPIO as GPIO
PIN = 'P9_23'
print "Setting up pin" + PIN + " as an input to trigger off of hop frame indicator..."
GPIO.setup(PIN, GPIO.IN)

# Busy sleep is more accurate on a non-realtime kernel!
def busy_sleep(dt):   
	current_time = time()
	stop_time = current_time+dt
	while (time() < stop_time):
		pass

def test():
	sleeptimes = numpy.linspace(11,11,256) # ms
	trials = len(sleeptimes)
	print "Will do " + str(trials) + " trials"
	actual_sleeptimes = numpy.ones(trials)*numpy.nan
	i = 0
	while i<trials:
		#print "Waiting for hop frame indicator..."
		GPIO.wait_for_edge(PIN, GPIO.FALLING)
		t0 = time()
		busy_sleep(sleeptimes[i]*.001)
		t00 = time()
		actual_sleeptimes[i] = (t00-t0) * 1000.0 # ms
		i += 1

	wakeup_times = actual_sleeptimes - sleeptimes
	best = min(wakeup_times)
	worst = max(wakeup_times)
	median = numpy.median(wakeup_times)
	print "Wakeup time (ms) Best: " + str(best) + " Median: " + str(median) + " Worst: " + str(worst)


# This makes a difference!
os.system('cpufreq-set -g performance')
sleep(300*.000001) # cpufreq-info says the transition latency is 300 us.
print "With governor set to performance:"
test()

# This makes a difference too!
pid = os.getpid()
prio = 99
os.system('chrt -f -p ' + str(prio) + ' ' + str(pid))
print "Additionally with FIFO realtime scheduler, prio " + str(prio) + ': '
test()

# Save some power
os.system('cpufreq-set -g ondemand')


