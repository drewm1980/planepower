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

set_autoconfig(ser1,False)
set_autoconfig(ser2,False)

ser1.close()
ser2.close()

