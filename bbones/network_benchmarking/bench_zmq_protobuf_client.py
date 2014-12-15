#!/usr/bin/env python

#   Hello World client in Python
#   Connects REQ socket to tcp://localhost:5555
#   Sends "Hello" to server, expects "World" back

import zmq
from time import time

context = zmq.Context()

#  Socket to talk to server
print("Connecting to hello world server...")
socket = context.socket(zmq.REQ)
#socket.connect("tcp://localhost:5555")
socket.connect("tcp://bbenet:5555")

#  Do 10 requests, waiting each time for a response
for request in range(1000):
    #print("Sending request %s ..." % request)

    t1 = time()
    socket.send(b"Hello")
    #  Get the reply.
    message = socket.recv()
    t2 = time()

    print("Received reply %s [ %s ], took %f sec" % (request, message,t2-t1))
