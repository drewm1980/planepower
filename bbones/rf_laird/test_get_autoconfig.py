#!/usr/bin/env python

# Try to disable autoconfigure in EEPROM on the radios, so that we can set
# the packet size on the fly or in EEPROM

import serial
from rm024 import *
from rm024_atcommands import *

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

if get_autoconfig(ser1):
	print "Serial device 1 has autoconfig enabled!"
else:
	print "Serial device 1 has autoconfig disabled!"

if get_autoconfig(ser2):
	print "Serial device 2 has autoconfig enabled!"
else:
	print "Serial device 2 has autoconfig disabled!"

ser1.close()
ser2.close()

