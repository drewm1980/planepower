#!/usr/bin/env python

import serial
from rm024 import test_device_alive

ser = serial.Serial(
		port='/dev/ttyO2',
		baudrate=115200,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS
		)

ser.open()
test_device_alive(ser)
print "Device seems to be alive!"
ser.close()

