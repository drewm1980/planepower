#!/usr/bin/env python
import random
import string
from time import time, sleep
import numpy
import os
import realtime

# Currently BROKEN!!!
def read_angle_sensors_hardware():
	from Adafruit_BBIO.SPI import SPI
	# For some reason, only the first one of these will
	# trigger whatever mechanism is using the device files
	spi1 = SPI(0,0)	#/dev/spidev1.0
	spi2 = SPI(0,1)	#/dev/spidev1.1
	spi3 = SPI(1,0)	#/dev/spidev2.0
	spi4 = SPI(1,1)	#/dev/spidev2.1
	spis = [spi1, spi2]
	from numpy import uint8, uint16
	spi = spi1
	ab = spi.readbytes(2)	
	a = uint16(ab[0])
	b = uint16(ab[1])
	#print bin(a),bin(b)
	print "{0:08b}".format(a), "{0:08b}".format(b)
	realtime.busy_sleep(.02)

read_angle_sensors_hardware()


