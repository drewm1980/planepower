#!/usr/bin/env python

import sys, time

from PyQt4 import QtCore, QtGui

import numpy as np

import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

import DynamicMheTelemetry_pb2 as mheProto

from vis_helpers import OcpWorker

app = QtGui.QApplication([])

class CustomGraphicsView( pg.GraphicsView ):
	def __init__(self, timer, parent = None):
		pg.GraphicsView.__init__(self, parent)

		assert isinstance(timer, QtCore.QTimer)
		self._timer = timer
	
	def keyPressEvent(self, event):
		if event.key() == QtCore.Qt.Key_P:
			# A simple start/stop of a timer if the key "P" is pressed
			if self._timer.isActive() is True:
				self._timer.stop()
			else:
				self._timer.start()

layout = pg.GraphicsLayout(border = (100, 100, 100))

#
# Generic fields, that all protobufs _must_ have
#
genNames = ["ts_trigger", "ts_elapsed"]

#
# Test fields
#

#
# Window organization
#
mhePlots = dict()

# x, y, z
posNames = [("x", "m"), ("y", "m"), ("z", "m")]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), posNames, posNames) )
# RPY, or e11_...e33
rpyNames = [("roll", "deg"), ("pitch", "deg"), ("yaw", "deg")]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), rpyNames, rpyNames) )
# dx, dy, dz
velNames = [("dx", "m/s"), ("dy", "m/s"), ("dz", "m/s")]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), velNames, velNames) )
# w_... x, y, z
gyroNames = [("w_bn_b_x", "rad/s"), "w_bn_b_y", "w_bn_b_z"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), gyroNames, gyroNames) )

layout.nextRow()

# aileron, elevator; daileron, delevator
ctrlNames = [("aileron", "deg"), ("daileron", "deg/s"), "elevator", "delevator"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), ctrlNames, ctrlNames) )
# ddelta, motor_torque, dmotor_torque, [cos, sin delta]
carNames = [("ddelta", "rpm"), ("motor_torque", "Nm"), ("dmotor_torque", "Nm/s")]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), carNames, carNames) )
# r, dr, ddr, dddr
cableNames = [("r", "m"), ("dr", "m/s"), ("ddr", "m/s^2"), ("dddr", "m/s^3")]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), cableNames, cableNames) )
# obj_value, kkt_value, exec_prep, exec_fdb
perfNames = ["obj_value", "kkt_value", ("exec_prep", "s"), ("exec_fdb", "s")]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), perfNames, perfNames,
				options = {"obj_value": ["semilogy"],
						   "kkt_value": ["semilogy"]}) )

horizonNamesAlt = posNames + rpyNames + velNames + gyroNames + \
				  ctrlNames + carNames + cableNames
historyNamesAlt = perfNames

horizonNames = []
for v in horizonNamesAlt:
	if isinstance(v, tuple):
		horizonNames.extend( [ v[ 0 ] ] )
	else:
		assert isinstance(v, str)
		horizonNames.extend( [ v ] )
historyNames = []
for v in historyNamesAlt:
	if isinstance(v, tuple):
		historyNames.extend( [ v[ 0 ] ] )
	else:
		assert isinstance(v, str)
		historyNames.extend( [ v ] )

#
# Setup update of the plotter
#

# Queue for data exchange between the worker and the main thread
import Queue
q1 = Queue.Queue(maxsize = 10)

def updatePlots():
	global q1
	global mhePlots
	global horizonNames, historyNames

	def updateGroup(q, plots):
		try:
			data = q.get_nowait()
			timeStamps = data[ "ts_trigger" ]
			# Update 
			map(lambda name: plots[ name ].setData( data[ name ] ), horizonNames)
			map(lambda name: plots[ name ].setData(timeStamps, data[ name ]), historyNames)
			
		except Queue.Empty:
			pass
		
	# Update all plots
	updateGroup(q1, mhePlots)

timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 100 )

mheNamesExt = genNames + historyNames + horizonNames

#
# ZMQ part:
#

host = "192.168.1.110"
#host = "localhost"

DynamicMhePort = "5570"

# Create workers
workers = []

workers.append(OcpWorker(mheProto, host + ":" + DynamicMhePort, q1, bufferSize = 20 * 25))

# Start Qt event loop unless running in interactive mode.
#
if __name__ == '__main__':

	view = CustomGraphicsView( timer )
	view.setCentralItem( layout )
	view.show()
	view.setWindowTitle(
		"Telemetry for MHE; Horizons of states and controls and history of performance indicators" )
	view.resize(1024, 768)

	for worker in workers:
		worker.start()
	
	import sys
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()
	
	for worker in workers:
		worker.stop()
