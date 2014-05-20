#!/usr/bin/env python
from time import time, sleep
import zmq

import realtime

from bitbang_spi_wrapper import read_angle_sensors

# Set up protobuf serialization
from LineAngleSensor2Telemetry_pb2 import LineAngleSensor2Msg
m = LineAngleSensor2Msg()

# Set up zeromq 
con = zmq.Context(20) # Takes number of i/o threads... not sure if these will inherit our realtime permissions...
soc = con.socket(zmq.PUB)
soc.bind('tcp://127.0.0.1:5555')

#realtime.enable()
t0 = time()
while True:
	sleep(.1)

	t1 = time()
	az,el = read_angle_sensors()
	t2 = time()-t1

	m.angle_hor,m.angle_ver = az,el
	#string = m.SerializeToString()
	t3 = time()
	s = m.SerializePartialToString() # Has less error checking
	t4 = time()-t3
	

	print "Sending %f,%f"%(az,el)
	t5 = time()
	soc.send(s)
	t6 = time() - t5

	print ' '.join(map(str,(t2,t4,t6)))


#realtime.disable()


