#!/usr/bin/env python
import sys, time

from PyQt4 import QtCore, QtGui
import numpy as np
import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

sys.path.append('../components/carousel2/SiemensSensors/types')
sys.path.append('../components/carousel2/SiemensActuators/types')

protobufs = ["SiemensDriveState", "SiemensDriveCommand"]
for p in protobufs:
	exec "from %sTelemetry_pb2 import %sMsg" % (p, p)

app = QtGui.QApplication([])

layout = pg.GraphicsLayout(border = (100, 100, 100))
view = pg.GraphicsView()
view.setCentralItem( layout )
view.show()
view.setWindowTitle( "HIGHWIND Carousel2 Telemetry" )
view.resize(1024, 768)

# Generic fields, that all protobufs _must_ have
genNames = ["ts_trigger", "ts_elapsed"]

# MCU handler structure init
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

# Extended name lists
mcuNamesExt = mcuNames + genNames
encNamesExt = encNames + genNames
winchNamesExt = winchNames + genNames
lasNamesExt = lasNames + genNames
ledNamesExt = ledNames + genNames

# Setup update of the plotter

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
		
	# Update plots
	updateGroup(q1, mcuPlots, mcuNames)
	updateGroup(q2, encPlots, encNames)
	updateGroup(q3, winchPlots, winchNames)
	updateGroup(q4, lasPlots, lasNames)

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

# Start Qt event loop unless running in interactive mode.
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
