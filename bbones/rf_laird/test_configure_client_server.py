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

SET_TO_CLIENT={"call":"\x}
        [SET_TO_CLIENT].callSize = 3, [SET_TO_CLIENT].call = {0xCC,0x03,0x03},
        [SET_TO_CLIENT].respSize = 1, [SET_TO_CLIENT].response = {0xCC},
        [SET_TO_CLIENT].argSize = 0,  [SET_TO_CLIENT].dataSize = 2,

        [SET_TO_SERVER].callSize = 3, [SET_TO_SERVER].call = {0xCC,0x03,0x02},
        [SET_TO_SERVER].respSize = 1, [SET_TO_SERVER].response = {0xCC},
        [SET_TO_SERVER].argSize = 0,  [SET_TO_SERVER].dataSize = 2,
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

