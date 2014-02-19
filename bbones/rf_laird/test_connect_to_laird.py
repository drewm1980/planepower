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

ENTER_COMMAND_MODE_call='\x41\x54\x2B\x2B\x2B\x0D'
ENTER_COMMAND_MODE_response='\xCC\x43\x4F\x4D'
EXIT_COMAND_MODE_call='\xCC\x41\x54\x4F\x0D'
EXIT_COMAND_MODE_response='\xCC\x44\x41\x54'

ser.write(ENTER_COMMAND_MODE_call)
ser.flushOutput()
response = ser.read(len(ENTER_COMMAND_MODE_response))
if response == ENTER_COMMAND_MODE_response:
	print "Successfully entered command mode!"
else:
	print "Unable to enter command mode!"
	print "Return value was:" + str(response)


ser.close()

