#!/usr/bin/env python

import sys, time

from PyQt4 import QtCore, QtGui

import numpy as np

import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

from DynamicMheTelemetry_pb2 import DynamicMheMsg

app = QtGui.QApplication([])

layout = pg.GraphicsLayout(border = (100, 100, 100))
view = pg.GraphicsView()
view.setCentralItem( layout )
view.show()
view.setWindowTitle( "Telemetry for MHE" )
view.resize(1024, 768)

#
# Generic fields, that all protobufs _must_ have
#
genNames = ["ts_trigger", "ts_elapsed"]

#
# Test fields
#
#
# Control surfaces
#
testTitle = "TEST"

mheNames = ["exec_fdb", "exec_prep"]
mhePlots = addPlotsToLayout(layout.addLayout( ), testTitle, mheNames)

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
			map(lambda name: plots[ name ].setData(timeStamps, data[ name ] ), names)
			
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

workers.append(ZmqSubProtobufWorker(host + ":" + DynamicMhePort, DynamicMheMsg, mheNamesExt,
									q1, bufferSize = 20 * 25))

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
