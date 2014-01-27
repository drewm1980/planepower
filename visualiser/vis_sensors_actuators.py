#!/usr/bin/env python

import sys, time

from PyQt4 import QtCore, QtGui

import numpy as np

import pyqtgraph as pg

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

colors = ["#FF0000", "#ADFF2F", "#00BFFF", "#FFFF00", "#FAA460"]

def redBoldText( txt ):
	t = "<span style='color: " + colors[ 0 ] + "; font-weight: bold'>" + str( txt ) + "</span>"
	return t

def greenBoldText( txt ):
	t = "<span style='color: " + colors[ 1 ] + "; font-weight: bold'>" + str( txt ) + "</span>"
	return t

def blueBoldText( txt ):
	t = "<span style='color: " + colors[ 2 ] + "; font-weight: bold'>" + str( txt ) + "</span>"
	return t

def addPlotsToLayout(layout, title, names):
	layout.setContentsMargins(10, 10, 10, 10)
	layout.addLabel( title )
	layout.nextRow()
	
	d = dict()
	for k, v in enumerate( names ):
		plt = layout.addPlot()
		if k < len( names ) - 1:
			plt.hideAxis( "bottom" )
			layout.nextRow()
		
		d[ v ] = plt.plot()
		d[ v ].setPen( colors[ k ] )

	return d

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
ledNames =  []
for frame in ["left", "right"]:
	for clr in ["r", "g", "b"]:
		for coord in ["u", "v"]:
			ledNames.append(frame + "_" + clr + "_" + coord)

ledNamesExt = ledNames + ["ts_trigger", "ts_elapsed"]

ledPlots = dict()
for f in ["left", "right"]:
	for coord in ["u", "v"]:
		names = []
		for clr in ["r", "g", "b"]:
			names.append(f + "_" + clr + "_" + coord)
		
		plots = addPlotsToLayout(ledLayout.addLayout(),
								 f + " frame, coord: " + coord, names)

		ledPlots.update( plots )

# TODO does not work yet
for col in xrange( 4 ):
	l = ledLayout.getItem(0, col)
	for row in xrange( 3 ):
		a = l.getItem(row + 1, 0).getAxis( "left" )
		a.setRange(-1, 1200)
		a.linkToView(l.getItem(row + 1, 0).getViewBox())

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

	# TODO Use this guy to make code shorter
	#      but first create mcuPlots
	def updateGroup(q, plots, names):
		try:
			data = q.get_nowait()
			map(lambda name: plots[ name ].setData( data[ name ] ), names)
			
		except:
			Queue.Empty
		
	# Update all plots
	updateGroup(q1, mcuPlots, mcuNames)
	updateGroup(q2, encPlots, encNames)
	updateGroup(q3, winchPlots, winchNames)
	updateGroup(q4, lasPlots, lasNames)
	updateGroup(q5, ledPlots, ledNames)

timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 10 )

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

workers.append(ZmqSubProtobufWorker(host + ":" + McuHandlerPort, McuHandlerMsg, gyroNames + acclNames + ctrlNames,
									q1, bufferSize = 20 * 100))

workers.append(ZmqSubProtobufWorker(host + ":" + EncoderPort, EncoderMsg, encNames,
									q2, bufferSize = 20 * 100))

workers.append(ZmqSubProtobufWorker(host + ":" + WinchPort, WinchControlMsg, winchNames,
									q3, bufferSize = int(20 * 12.5)))

workers.append(ZmqSubProtobufWorker(host + ":" + LasPort, LineAngleSensorMsg, lasNames,
									q4, bufferSize = 20 * 100))

workers.append(LedTrackerWorker(host + ":" + LedTrackerPort, LEDTrackerMsg, ledNamesExt,
									q5, bufferSize = int(20 * 12.5)))

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
