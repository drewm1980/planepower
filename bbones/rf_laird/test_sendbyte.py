#!/usr/bin/env python

import serial
ser = serial.Serial(
		port='/dev/ttyO2',
		baudrate=115200,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS
		)

ser.open()
ser.isOpen()

#msg = chr(0b10101010)
msg = chr(0b01010101)
ser.write(msg)
ser.flushOutput()
foo = ser.read(len(msg))
print foo

print "For 9600 baud, the pulse width (bit width) you see should be 104 us"

ser.close()

