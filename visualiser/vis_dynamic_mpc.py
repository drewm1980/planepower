#!/usr/bin/env python

import sys, time

from PyQt4 import QtCore, QtGui

import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

import DynamicMpcTelemetry_pb2 as mpcProto

from vis_helpers import OcpWorker

app = QtGui.QApplication([])

layout = pg.GraphicsLayout(border = (100, 100, 100))
view = pg.GraphicsView()
view.setCentralItem( layout )
view.show()
view.setWindowTitle(
	"Telemetry for NMPC" )
view.resize(1024, 768)

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
mpcPlots = dict()

# x, y, z; plus references
posNames = [[("x", "m"), "y_x"],
			[("y", "m"), "y_y"],
			[("z", "m"), "y_z"]]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), posNames, posNames) )
# RPY, or e11_...e33
rpyNames = [("roll", "deg"), ("pitch", "deg"), ("yaw", "deg")]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), rpyNames, rpyNames) )
# dx, dy, dz
velNames = [("dx", "m/s"), ("dy", "m/s"), ("dz", "m/s")]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), velNames, velNames) )
# w_... x, y, z
gyroNames = [[("w_bn_b_x", "rad/s"), "y_w_bn_b_x"],
			 ["w_bn_b_y", "y_w_bn_b_y"],
			 ["w_bn_b_z", "y_w_bn_b_z"]]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), gyroNames, gyroNames) )

layout.nextRow()

# aileron, elevator; daileron, delevator
uaNames = [[("aileron", "deg"), "y_aileron"],
		   [("daileron", "deg/s"), "y_daileron"]]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), uaNames, uaNames) )
ueNames = [[("elevator", "deg"), "y_elevator"],
		   [("delevator", "deg/s"), "y_delevator"]]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), ueNames, ueNames) )
# obj_value, kkt_value, exec_prep, exec_fdb
perfNames = ["n_asc", "obj_value", "kkt_value"]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), perfNames, perfNames,
				options = {"obj_value": ["semilogy"],
						   "kkt_value": ["semilogy"]}) )
execNames = [("exec_prep", "s"), ("exec_fdb", "s")]
mpcPlots.update( addPlotsToLayout(layout.addLayout( ), execNames, execNames) )

horizonNamesAlt = flatten(posNames + rpyNames + velNames + gyroNames + \
				  uaNames + ueNames)
# 				  + carNames + cableNames
historyNamesAlt = perfNames + execNames

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
	global mpcPlots
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
	updateGroup(q1, mpcPlots)

timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 100 )

#
# ZMQ part:
#

host = "192.168.1.110"

DynamicMpcPort = "5571"

# Create workers
workers = []

workers.append(OcpWorker(mpcProto, host + ":" + DynamicMpcPort, q1, bufferSize = 20 * 25))

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
