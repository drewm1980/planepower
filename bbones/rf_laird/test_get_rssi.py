#!/usr/bin/env python

# Set two connected devices, one to server, one to client, check status

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

while 1:
	get_last_rssi(ser1)
	get_last_rssi(ser2)

ser1.close()
ser2.close()

