#!/usr/bin/env python

# Send a random over two locally connected serial modems,
# benchmark transmission latency

import serial
from rm024 import *
from rm024_atcommands import *
import random
import string
from time import time, sleep
import numpy
import realtime

#timeout = .015 # s
timeout = 1 # s
ser1 = serial.Serial(
		port='/dev/ttyO1',
		baudrate=115200,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		timeout=timeout
		)
ser1.open()
ser2 = serial.Serial(
		port='/dev/ttyO4',
		baudrate=115200,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		timeout=timeout
		)
ser2.open()

import Adafruit_BBIO.GPIO as GPIO
PIN = 'P9_23'
print "Setting up pin" + PIN + " as an input to trigger off of hop frame indicator..."
GPIO.setup(PIN, GPIO.IN)

# worst case packet size
imu_bytes = 6*4
line_angle_bytes = 2*2
servo_bytes = 7*2
packet_size = imu_bytes + line_angle_bytes + servo_bytes

packet_size = 17

set_rf_packet_size_on_fly(ser1,packet_size)
set_rf_packet_size_on_fly(ser2,packet_size)

print "Packet size for test: " + str(packet_size) + " bytes"

def randomword(length):
	   return ''.join(random.choice(string.lowercase) for i in range(length))

bestCase = 999999999
worstCase = -1
best_intended_sleep_time = 999999999999999999

lastbest_intended = 12
margin = .1
intended_sleeptimes = numpy.linspace(lastbest_intended-margin,lastbest_intended-margin,256)
intended_sleeptimes = numpy.linspace(0,13.2,256) # ms

# The choice of sender doesn't seem to make a difference:
#sender,reciever=ser1,ser2
sender,reciever=ser2,ser1

realtime.enable()

trials = len(intended_sleeptimes)
print "Will do " + str(trials) + " trials"
latencies = numpy.ones(trials)*numpy.nan
actual_sleeptimes = numpy.ones(trials)*numpy.nan
i = 0
recieved = 0
dropped = 0
while i<trials:
	reciever.flushInput()
	msg=randomword(packet_size)

	#print "Waiting for hop frame indicator..."
	GPIO.wait_for_edge(PIN, GPIO.FALLING)

	t0 = time()
	realtime.busy_sleep(intended_sleeptimes[i]*.001)
	t00 = time()
	actual_sleeptimes[i] = (t00-t0) * 1000.0

	t1 = time()
	sender.write(msg)
	sender.flushOutput()

	# RF latency happens here

	response = reciever.read(len(msg))
	if not (msg == response):
		print "Error in packet transmission!"
		print "Packet size: " + str(len(msg))
		print "Sent " + str(len(msg)) + " bytes:\t\t" + msg
		print "Recieved " + str(len(response)) + " bytes:\t" + response
		
		dropped += 1
		continue
	recieved += 1
	t2 = time()
	latency = (t2-t1)*1000 # ms
	latencies[i] = latency
	if latency < bestCase:
		bestCase = latency
		updated = 1
		best_intended_sleep_time = intended_sleeptimes[i]
	if latency > worstCase:
		worstCase = latency
		updated = 1
	if updated:
		print "Trial: " + str(i+1) + " Latency (ms) Best: " + str(bestCase) + " Last: " + str(latency) + " Worst: " + str(worstCase)
		updated = 0
	i += 1

realtime.disable()

print "Trial: " + str(i+1) + " Latency (ms) Best: " + str(bestCase) + " Median: " + str(numpy.median(latencies)) + " Worst: " + str(worstCase)

print "Recieved: " + str(recieved) + " Dropped: " + str(dropped)

print "Best (intended) sleep time was: " + str(best_intended_sleep_time)

ser1.close()
ser2.close()

import pickle
foo = actual_sleeptimes,latencies
pickle.dump(foo,open('latencies.pickle','wb'))

import os
os.system('scp latencies.pickle awagner@192.168.7.1:planepower/bbones/rf_laird/')
