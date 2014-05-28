#!/usr/bin/env python
import signal
import sys, time, signal

from PyQt4 import QtCore, QtGui
import numpy as np
import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

paths=['SiemensSensors','siemensSensors','siemensActuators','resampler','lineAngleSensor2']
for p in paths:
    paath = '../components/carousel2/' + p + '/types'
    print "Adding path " + paath + " to the python search path..."
    sys.path.append(paath)

print "Importing protobufs..."
protobufs = ["LineAngles", "ResampledMeasurements", "SiemensDriveState", "SiemensDriveCommand",]
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
#gyroTitle = "Gyroscope data [rad/s] (" + \
             #redBoldText("X") + ", " + greenBoldText("Y") + \
             #", " + blueBoldText("Z") + \
             #") vs Timestamp [s]"

#gyroNames = ["gyro_x", "gyro_y", "gyro_z"]
#mcuPlots.update( addPlotsToLayout(layout.addLayout( ), gyroTitle, gyroNames) )
#mcuNames.extend( gyroNames )

# Line angle sensor measurements
lasTitle = "Line angle sensor data [Radians] (" + \
           redBoldText("azimuth") + ", " + greenBoldText("elevation") + \
           ") vs Timestamp [s]"

lasNames = ["azimuth", "elevation"] # These are members in the struct, apparently
lasPlots = addPlotsToLayout(layout.addLayout(), lasTitle, lasNames)
lasNamesExt = lasNames + genNames

# Setup update of the plotter

# Queue for data exchange between the worker and the main thread
import Queue
q1 = Queue.Queue(maxsize = 10)
#q2 = Queue.Queue(maxsize = 10)
#q3 = Queue.Queue(maxsize = 10)
#q4 = Queue.Queue(maxsize = 10)
#q5 = Queue.Queue(maxsize = 10)

def updatePlots():
    global q1
    #, q2, q3, q4, q5
    #global mcuPlots, encPlots, winchPlots, lasPlots, ledPlots
    #global mcuNames, encNames, winchNames, lasNames, ledNames
    global lasPlots, lasNames

    def updateGroup(q, plots, names):
        try:
            data = q.get_nowait()
            timeStamps = data[ "ts_trigger" ]
            map(lambda name: plots[ name ].setData(timeStamps, data[ name ] ), names)
            
        except Queue.Empty:
            pass
        
    # Update plots
    updateGroup(q1, lasPlots, lasNames)

# Set up a timer to update the plots
timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 100 ) # Tick in [ms]

def signal_handler(signal, frame):
        print('You pressed Ctrl+C!')
        sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

#
# ZMQ part:
#

host = "localhost"
#host = "192.168.1.110"

# This MUST match the order in load_telemetry.lua,
# otherwise the ports will not be enumerated correctly!
telemetryInstanceNames=["siemensSensorsTelemetry",      
                        "lineAngleSensor2Telemetry", 
                        "resampledMeasurementsTelemetry", 
                        "controllerTelemetry"]
ports = dict() 
startPort = 5562
for i in range(len(telemetryInstanceNames)):
    portnumber = startPort+i
    print "Port for "+telemetryInstanceNames[i]+" is "+str(portnumber)
    ports[telemetryInstanceNames[i]] = str(portnumber)
    
#McuHandlerPort = "5563"
#EncoderPort    = "5564"
#LedTrackerPort = "5565"
#LasPort        = "5566"
#WinchPort      = "5567"

# Create workers
workers = []

# NOTE: All buffer lenghts are set to correspond to last 20 sec.
# TODO: Somehow we should automate this...

workers.append(ZmqSubProtobufWorker(host + ":" + ports['lineAngleSensor2Telemetry'], LineAnglesMsg, lasNamesExt,
                                    q1, bufferSize = 20 * 100))

# Start Qt event loop unless running in interactive mode.
if __name__ == '__main__':

    #
    # Testing
    #
#    for p in gyroNames:
#        gyroPlots[ p ].setData( np.random.normal(size = 1000, scale = 0.1) )

    for worker in workers:
        worker.start()
    
    import sys
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()
    
    for worker in workers:
        worker.stop()
