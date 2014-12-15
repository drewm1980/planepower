#!/usr/bin/env python

# See readme_realtime.txt

from time import time, sleep
import os

# Busy sleep is more accurate on a non-realtime kernel!
def busy_sleep(dt):   
	current_time = time()
	stop_time = current_time+dt
	while (time() < stop_time):
		pass

# Tune the cpu and thread priorities to improve wakup time
# This stuff works even on a non-realtime kernel
def enable():
	os.system('cpufreq-set -g performance') # Max out cpu clock speed
	pid = os.getpid()
	prio = 51
	# Set FIFO scheduler, realtime priority
	os.system('chrt -f -p ' + str(prio) + ' ' + str(pid)) 
	sleep(300*.000001) # cpufreq-info says the transition latency is 300 us.

# Return the system to normal
def disable():
	os.system('cpufreq-set -g ondemand')
	pid = os.getpid()
	os.system('chrt -o -p ' + str(pid))
	sleep(300*.000001) # cpufreq-info says the transition latency is 300 us.


