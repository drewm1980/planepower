#!/usr/bin/env python

# Send "Hello World!" over two locally connected serial modems

import serial
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

msg = "Hello World!"

ser1.write(msg)
ser1.flushOutput()
response = ser2.read(len(msg))
if response == msg:
	print "Successfully sent Hello World wirelessly!"
else:
	print "Recieved string doesn't match sent string!"
	print "Return value was:" + str(response)

ser1.close()
ser2.close()

