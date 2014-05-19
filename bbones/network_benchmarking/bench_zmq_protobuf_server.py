#!/usr/bin/env python

#  Hello World server

import zmq
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

while True:
	print "Waiting for message from client!"
	message = socket.recv()
	print "Sending response!"
	socket.send("World")

