#!/usr/bin/env python
from time import time, sleep
import zmq

import realtime

# Set up protobuf serialization
from LineAngleSensor2Telemetry_pb2 import LineAngleSensor2Msg
#m = LineAngleSensor2Msg()

# Set up zeromq 
con = zmq.Context(20) # Takes number of i/o threads... not sure if these will inherit our realtime permissions...
soc = con.socket(zmq.SUB)
soc.connect('tcp://127.0.0.1:5555')
soc.setsockopt(zmq.SUBSCRIBE,"") # The string is doing pattern matching on the first bytes of the string, it seems...

realtime.enable()
t0 = time()
while True: 
        sleep(.00001)
        t1 = time()-t0

	print "Waiting for a message..."
	s = soc.recv()

	m = LineAngleSensor2Msg.FromString(s)

	az,el = m.angle_hor,m.angle_ver
	print az,el

        t2 = time()-t0
realtime.disable()



