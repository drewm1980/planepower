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

#send_at_command(ser1,SET_TO_SERVER)
#send_at_command(ser2,SET_TO_CLIENT)
#send_at_command(ser1,EEPROM_SET_SERVER)
#send_at_command(ser2,EEPROM_SET_CLIENT)
set_to_server_on_fly(ser1)
set_to_client_on_fly(ser2)

ser1.close()
ser2.close()

