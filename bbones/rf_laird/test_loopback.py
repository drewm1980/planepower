#!/usr/bin/env python

import serial
ser = serial.Serial(
		port='/dev/ttyO2',
		baudrate=9600,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS
		)

ser.open()
ser.isOpen()

msg = "If you are reading this, then the loopback test worked!"
ser.write(msg)
foo = ser.read(len(msg))
print foo

ser.close()

