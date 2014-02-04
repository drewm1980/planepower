#!/usr/bin/env python

import sys, time

from PyQt4 import QtCore, QtGui

import numpy as np

import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

import DynamicMheTelemetry_pb2 as mheProto

from collections import deque

app = QtGui.QApplication([])

layout = pg.GraphicsLayout(border = (100, 100, 100))
view = pg.GraphicsView()
view.setCentralItem( layout )
view.show()
view.setWindowTitle( "Telemetry for MHE" )
view.resize(1024, 768)

#
# Deserialization
# TODO in protobuf version 2.5.0 this can be achieved a bit easier
#

xNames = mheProto._DYNAMICMHEMSG_XNAMES
#zNames = DynamicMheTelemetry_pb2._DYNAMICMHEMSG_ZNAMES
#uNames = DynamicMheTelemetry_pb2._DYNAMICMHEMSG_UNAMES

class DynamicMheWorker( ZmqSubProtobufWorker ):
	def __init__(self, address, queue, bufferSize = 20, topic = ""):
		
		# Make the xNames enum dictionary, while removing "idx_" from keys 
		self._xMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in xNames.values)

		self._simpleFieldNames = ["solver_status", "kkt_value", "obj_value",
								  "exec_fdb", "exec_prep", "ts_trigger", "ts_elapsed"]

		ZmqSubProtobufWorker.__init__(self, address, mheProto.DynamicMheMsg, self._simpleFieldNames,
									  queue, bufferSize, topic)
		
		# Define new buffers
		for k in self._xMap.keys():
			self._buffer[ k ] = list( xrange( self._msg.N ) )

#		self._zHorizons = dict()
#		self._uHorizons = dict()

	def deserialize( self ):
		def updateBufferSimple( name ):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( getattr(self._msg, name) )

		map(updateBufferSimple, self._simpleFieldNames)

		def updateHorizonBuffers(nMap, name):
			el = getattr(self._msg, name)
			for k, v in nMap.items():
				self._buffer[ k ] = el[ v ].h._values

		updateHorizonBuffers(self._xMap, "x")

#
# Generic fields, that all protobufs _must_ have
#
genNames = ["ts_trigger", "ts_elapsed"]

#
# Test fields
#

testTitle = "x, y, z [m]"

mheNames = ["x", "y", "z"]
mhePlots = addPlotsToLayout(layout.addLayout( ), testTitle, mheNames)

#
# Window organization
#

# x, y, z
# RPY, or e11_...e33
# dx, dy, dz
# w_... x, y, z
# aileron, elevator; daileron, delevator
# ddelta, motor_torque, dmotor_torque, [cos, sin delta]
# r, dr, ddr, dddr
# obj, kkt, exec_prep, exec_fdb

#
# Setup update of the plotter
#

# Queue for data exchange between the worker and the main thread
import Queue
q1 = Queue.Queue(maxsize = 10)

def updatePlots():
	global q1
	global mhePlots
	global mheNames

	def updateGroup(q, plots, names):
		try:
			data = q.get_nowait()
			timeStamps = data[ "ts_trigger" ]
			#map(lambda name: plots[ name ].setData(timeStamps, data[ name ] ), names)
			map(lambda name: plots[ name ].setData( data[ name ] ), names)
			
		except Queue.Empty:
			pass
		
	# Update all plots
	updateGroup(q1, mhePlots, mheNames)

timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 100 )

mheNamesExt = mheNames + genNames

#
# ZMQ part:
#

host = "192.168.1.110"

DynamicMhePort = "5570"

# Create workers
workers = []

# NOTE: All buffer lenghts are set to correspond to last 20 sec.
# TODO: Somehow we should automate this...

#workers.append(ZmqSubProtobufWorker(host + ":" + DynamicMhePort, mheProto.DynamicMheMsg, mheNamesExt,
#									q1, bufferSize = 20 * 25))

workers.append(DynamicMheWorker(host + ":" + DynamicMhePort, q1, bufferSize = 20 * 25))

# Start Qt event loop unless running in interactive mode.
#
if __name__ == '__main__':

	for worker in workers:
		worker.start()
	
	import sys
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()
	
	for worker in workers:
		worker.stop()
