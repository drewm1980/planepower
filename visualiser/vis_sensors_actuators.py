#!/usr/bin/env python

import sys, time

from PyQt4 import QtCore, QtGui

import numpy as np

import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

protobufs = ["McuHandler", "Encoder", "WinchControl", "LineAngleSensor", "LEDTracker"]
for p in protobufs:
	exec "from %sTelemetry_pb2 import %sMsg" % (p, p)

app = QtGui.QApplication([])

layout = pg.GraphicsLayout(border = (100, 100, 100))
view = pg.GraphicsView()
view.setCentralItem( layout )
view.show()
view.setWindowTitle( "Telemetry - IMU, Control Surfaces, Encoder, Winch, Line Angle Sensor" )
view.resize(1024, 768)

ledLayout = pg.GraphicsLayout(border = (100, 100, 100))
ledView = pg.GraphicsView()
ledView.setCentralItem( ledLayout )
ledView.show()
ledView.setWindowTitle( "Telemetry - LED Tracker" )
ledView.resize(1024, 768)

#
# LED tracker deserialization, a very specific case
#

# Special deserialization of the LEDTracker data
class LedTrackerWorker( ZmqSubProtobufWorker ):
	def __init__(self, address, protobuf, names, queue, bufferSize = 20, topic = ""):
		ZmqSubProtobufWorker.__init__(self, address, protobuf, names,
									  queue, bufferSize, topic)

	def deserialize( self ):
		def updateBufferSimple( name ):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( getattr(self._msg, name) )

		def updateBuffer(name, val):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( val )

		map(updateBufferSimple, ["ts_trigger", "ts_elapsed"])

		if len(self._msg.frames) != 2:
			return;

		# TODO for i, v in enumerate( fr ): should work
		fr = ["left", "right"]
		for i in xrange( len( fr ) ):
			frame = self._msg.frames[ i ]

			for clr in ["r", "g", "b"]:
				c = getattr(frame, clr)
				for coord in ["u", "v"]:
					name = fr[ i ] + "_" + clr + "_" + coord
					val = getattr(c, coord)
					updateBuffer(name, val)

#
# Generic fields, that all protobufs _must_ have
#
genNames = ["ts_trigger", "ts_elapsed"]

#
# MCU handler structure init
#
mcuNames = []
mcuPlots = dict()

#
# Gyroscopes
#
gyroTitle = "Gyroscope data [rad/s] (" + \
			 redBoldText("X") + ", " + greenBoldText("Y") + \
			 ", " + blueBoldText("Z") + \
			 ") vs Timestamp [s]"

gyroNames = ["gyro_x", "gyro_y", "gyro_z"]
mcuPlots.update( addPlotsToLayout(layout.addLayout( ), gyroTitle, gyroNames) )
mcuNames.extend( gyroNames )

#
# Accelerations
#
acclTitle = "Accelerometer data [m/s^2] (" + \
			redBoldText("X") + ", " + greenBoldText("Y") + \
			", " + blueBoldText("Z") + \
			") vs Timestamp [s]"

acclNames = ["accl_x", "accl_y", "accl_z"]
mcuPlots.update( addPlotsToLayout(layout.addLayout( ), acclTitle, acclNames) )
mcuNames.extend( acclNames )

#
# Winch control
#
winchTitle = "Winch data (" + \
			redBoldText("t. len. [m]") + ", " + greenBoldText("t. sp. [m/s]") + \
			", " + blueBoldText("m. curr. [A]") + \
			") vs Timestamp [s]"

winchNames = ["length", "speed", "dbg_current"]
winchPlots = addPlotsToLayout(layout.addLayout( ), winchTitle, winchNames)

layout.nextRow()

#
# Control surfaces
#
ctrlTitle = "Control surfaces [rad] (" + \
			redBoldText("Aileron1") + ", " + greenBoldText("Aileron2") + \
			", " + blueBoldText("Elevator") + \
			") vs Timestamp [s]"

ctrlNames = ["ua1", "ua2", "ue"]
mcuPlots.update( addPlotsToLayout(layout.addLayout( ), ctrlTitle, ctrlNames) )
mcuNames.extend( ctrlNames )

#
# Encoder measurements
#
encTitle = "Encoder data (" + \
		   redBoldText("sin_angle") + ", " + greenBoldText("omega [rpm]") + \
		   ", " + blueBoldText("ref speed [V]") + \
		   ") vs Timestamp [s]"

encNames = ["sin_theta", "omega_filt_rpm", "dbg_speed_voltage"]
encPlots = addPlotsToLayout(layout.addLayout( ), encTitle, encNames)

#
# Line angle sensor measurements
#
lasTitle = "Line angle sensor data [?] (" + \
		   redBoldText("angle_ver") + ", " + greenBoldText("angle_hor") + \
		   ") vs Timestamp [s]"

lasNames = ["angle_ver", "angle_hor"]
lasPlots = addPlotsToLayout(layout.addLayout(), lasTitle, lasNames)

#
# LED Tracker data
#
ledNames = [frame + "_" + clr + "_" + coord
			for frame in ["left, right"]
			for clr in ["r", "g", "b"]
			for coord in ["u", "v"]]

def setupLedTrackerPlots( layout ):
	"""
	Customized way to plot LED tracker data, where we here plot (v, u) pairs
	for all markers.

	The function returns a dictionary of data plot items which we fill in
	with the data in the main thread.
	"""
	d = dict()

	class CustomPlotItem( pg.PlotItem ):
		def autoBtnClicked( self ):
			self.setXRange(0, 1200)
			self.setYRange(0, 1200)

	for frame in ["left", "right"]:
		l = layout.addLayout()
		l.setContentsMargins(10, 10, 10, 10)
		l.addLabel(frame + " frame, v vs u components")
		l.nextRow()

		plt = CustomPlotItem()
		l.addItem( plt )
		plt.setXRange(0, 1200)
		plt.setYRange(0, 1200)
		plt.showGrid(x = True, y = True)

		for k, clr in enumerate( ["r", "g", "b"] ):
			name = frame + "_" + clr
			
			symbols = ['o', 'd']
			for s, variant in enumerate( ["prev", "curr"] ):
				vName = name + "_" + variant
				# XXX Pay attention that here we use a scatter plot!
				item = pg.ScatterPlotItem()
				item.setSymbol( symbols[ s ] )

				if variant == "curr":
					# the current point is in color
					item.setBrush( colors[ k ] )
					item.setPen( colors[ k ] )
					item.setSize( 10 )
				else:
					# the past points are in grey
					item.setBrush( 0.5 )
					item.setPen( 0.5 )

				plt.addItem( item )
				d[ vName ] = item
	
	return d			

# Old way to plot LED tracker data, like all other plots			
# ledPlots = dict()
# for f in ["left", "right"]:
# 	for coord in ["u", "v"]:
# 		names = []
# 		for clr in ["r", "g", "b"]:
# 			names.append(f + "_" + clr + "_" + coord)
		
# 		plots = addPlotsToLayout(ledLayout.addLayout(),
# 								 f + " frame, coord: " + coord, names)

# 		ledPlots.update( plots )

ledPlots = setupLedTrackerPlots( ledLayout )

#
# Extended name lists
#

mcuNamesExt = mcuNames + genNames
encNamesExt = encNames + genNames
winchNamesExt = winchNames + genNames
lasNamesExt = lasNames + genNames
ledNamesExt = ledNames + genNames

#
# Setup update of the plotter
#

# Queue for data exchange between the worker and the main thread
import Queue
q1 = Queue.Queue(maxsize = 10)
q2 = Queue.Queue(maxsize = 10)
q3 = Queue.Queue(maxsize = 10)
q4 = Queue.Queue(maxsize = 10)
q5 = Queue.Queue(maxsize = 10)

def updatePlots():
	global q1, q2, q3, q4, q5
	global mcuPlots, encPlots, winchPlots, lasPlots, ledPlots
	global mcuNames, encNames, winchNames, lasNames, ledNames

	def updateGroup(q, plots, names):
		try:
			data = q.get_nowait()
			timeStamps = data[ "ts_trigger" ]
			map(lambda name: plots[ name ].setData(timeStamps, data[ name ] ), names)
			
		except Queue.Empty:
			pass
		
	# Update almost plots
	updateGroup(q1, mcuPlots, mcuNames)
	updateGroup(q2, encPlots, encNames)
	updateGroup(q3, winchPlots, winchNames)
	updateGroup(q4, lasPlots, lasNames)

	def updateLedPlots(q, plots):
		try:
			data = q.get_nowait()

			# TODO Optimize!
			for frame in ["left", "right"]:
				for clr in ["r", "g", "b"]:
					uName = frame + "_" + clr + "_u"
					vName = frame + "_" + clr + "_v"

					curr = frame + "_" + clr + "_curr"
					prev = frame + "_" + clr + "_prev"

					plots[ prev ].setData(data[ vName ], data[ uName ])
					plots[ curr ].setData([data[ vName ][ -1 ]], [data[ uName ][ -1 ]])
			
		except Queue.Empty:
			pass

	updateLedPlots(q5, ledPlots)
	# This is old way to plot LED tracker data, like all other plots...
#	updateGroup(q5, ledPlots, ledNames)

# Set up a timer to update the plots
timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 100 ) # Tick in [ms]

#
# ZMQ part:
#

#host = "localhost"
host = "192.168.1.110"

McuHandlerPort = "5563"
EncoderPort    = "5564"
LedTrackerPort = "5565"
LasPort        = "5566"
WinchPort      = "5567"

# Create workers
workers = []

# NOTE: All buffer lenghts are set to correspond to last 20 sec.
# TODO: Somehow we should automate this...

workers.append(ZmqSubProtobufWorker(host + ":" + McuHandlerPort, McuHandlerMsg, mcuNamesExt,
									q1, bufferSize = 20 * 100))

workers.append(ZmqSubProtobufWorker(host + ":" + EncoderPort, EncoderMsg, encNamesExt,
									q2, bufferSize = 20 * 100))

workers.append(ZmqSubProtobufWorker(host + ":" + WinchPort, WinchControlMsg, winchNamesExt,
									q3, bufferSize = 20 * 50))

workers.append(ZmqSubProtobufWorker(host + ":" + LasPort, LineAngleSensorMsg, lasNamesExt,
									q4, bufferSize = 20 * 100))

workers.append(LedTrackerWorker(host + ":" + LedTrackerPort, LEDTrackerMsg, ledNamesExt,
									q5, bufferSize = 20))

#
# Start Qt event loop unless running in interactive mode.
#
if __name__ == '__main__':

	#
	# Testing
	#
#	for p in gyroNames:
#		gyroPlots[ p ].setData( np.random.normal(size = 1000, scale = 0.1) )

	for worker in workers:
		worker.start()
	
	import sys
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()
	
	for worker in workers:
		worker.stop()
