#!/usr/bin/env python
import Adafruit_BBIO.GPIO as GPIO
 
PIN = 'P9_23'
GPIO.setup(PIN, GPIO.IN)
print "Done setting up pin" + PIN + " as an input"

while True:
	print "Waiting for interrupt..."
	GPIO.wait_for_edge(PIN, GPIO.RISING)
	print "Detected rising edge on pin " + PIN


