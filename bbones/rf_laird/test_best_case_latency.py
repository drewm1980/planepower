#!/usr/bin/env python

# Send "Hello World!" over two locally connected serial modems

import serial
from rm024 import *
from rm024_atcommands import *
import random
import string
from time import time, sleep
import numpy

ser1 = serial.Serial(
		port='/dev/ttyO2',
		baudrate=115200,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS
		)
ser1.open()
ser2 = serial.Serial(
		port='/dev/ttyO4',
		baudrate=115200,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS
		)
ser2.open()

# worst case packet size
imu_bytes = 6*4
line_angle_bytes = 2*2
servo_bytes = 7*2
msg_bytes = imu_bytes + line_angle_bytes + servo_bytes
msg_bytes = 16 # 32 not working for some reason...
set_rf_packet_size_on_fly(ser1,msg_bytes)
set_rf_packet_size_on_fly(ser2,msg_bytes)

print "Packet size for test: " + str(msg_bytes) + " bytes"

def randomword(length):
	   return ''.join(random.choice(string.lowercase) for i in range(length))

bestCase = 999999999
worstCase = -1

def random_sleep():
	t = random.uniform(0,.015)
	sleep(t) # up to 13 ms, i.e. the beacon frequency
	return t

# The choice of sender doesn't seem to make a difference:
#sender,reciever=ser1,ser2
sender,reciever=ser2,ser1

trials = 256
latencies = numpy.ones(trials)*numpy.nan
sleeptimes = numpy.ones(trials)*numpy.nan
for i in xrange(trials):
	reciever.flushInput()
	msg=randomword(msg_bytes)
	sleeptimes[i] = random_sleep() * 1000.
	t1 = time()
	sender.write(msg)
	sender.flushOutput()
	response = reciever.read(len(msg))
	t2 = time()
	latency = (t2-t1)*1000 # ms
	latencies[i] = latency
	if latency < bestCase:
		bestCase = latency
		updated = 1
	if latency > worstCase:
		worstCase = latency
		updated = 1
	if updated:
		print "Trial: " + str(i+1) + " Latency (ms) Best: " + str(bestCase) + " Last: " + str(latency) + " Worst: " + str(worstCase)
		updated = 0

print "Trial: " + str(i+1) + " Latency (ms) Best: " + str(bestCase) + " Median: " + str(numpy.median(latencies)) + " Worst: " + str(worstCase)

ser1.close()
ser2.close()

import pickle
foo = sleeptimes,latencies
pickle.dump(foo,open('latencies.pickle','wb'))

