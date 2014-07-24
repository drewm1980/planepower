#!/usr/bin/env python
import signal
import sys, time, signal

from PyQt4 import QtCore, QtGui
import numpy as np
import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

############### Import types #####################
paths=['siemensSensors','armboneLisaSensors','siemensActuators','resampler','lineAngleSensor2']
for p in paths:
    paath = '../components/carousel2/' + p + '/types'
    print "Adding path " + paath + " to the python search path..."
    sys.path.append(paath)

print "Importing protobufs..."
protobufs = ["LineAngles", "ResampledMeasurements", "SiemensDriveState", "SiemensDriveCommand","ImuGyro","ImuAccel","ImuMag"]
for p in protobufs:
    exec "from %sTelemetry_pb2 import %sMsg" % (p, p)

app = QtGui.QApplication([])

############## Network configuration and list of telemetry components  ###############
# This MUST match the order in load_telemetry.lua,
# otherwise the ports will not be enumerated correctly!
telemetryInstanceNames=["siemensSensorsTelemetry",      
                        "lineAngleSensor2Telemetry", 
                        "armboneGyroTelemetry",
                        "armboneAccelTelemetry",
                        "armboneMagTelemetry",
                        "resampledMeasurementsTelemetry", 
                        "controllerTelemetry"]

#host = "localhost" # DOES NOT WORK FOR SOME REASON!!!
host = "10.42.0.21" # This should be the IP address of the groundstation
ports = dict() 
startPort = 5562
for i in range(len(telemetryInstanceNames)):
    portnumber = startPort+i
    print "Port for "+telemetryInstanceNames[i]+" is "+str(portnumber)
    ports[telemetryInstanceNames[i]] = str(portnumber)

############## Setup layout #################
layout = pg.GraphicsLayout(border = (100, 100, 100))
view = pg.GraphicsView()
view.setCentralItem( layout )
view.show()
view.setWindowTitle( "HIGHWIND Carousel2 Telemetry" )
view.resize(5 * 1024, 756)

plotNames = ["las", "carousel", "controller","armboneGyro","armboneAccel","armboneMag"]

# Generic fields, that all protobufs _must_ have
genNames = ["ts_trigger", "ts_elapsed"]

# Line angle sensor measurements
lasTitle = "Line angle sensor data [Radians] (" + \
           redBoldText("azimuth") + ", " + greenBoldText("elevation") + \
           ") vs Timestamp [s]"
lasNames = ["azimuth", "elevation"] # These are members in the struct, apparently
lasNamesExt = lasNames + genNames
lasPlots = addPlotsToLayout(layout.addLayout(), lasTitle, lasNames)

# Carousel drive status
carouselTitle = "Carousel Drive Speed [(Arm)Radians/s] vs Timestamp [s]"
carouselNames = ["carouselSpeedSmoothed","carouselTorque"] # These are members in the struct, apparently
carouselNamesExt = carouselNames + genNames
carouselPlots = addPlotsToLayout(layout.addLayout(), carouselTitle, carouselNames)

# Controller measurements
controllerTitle = "Carousel Speed Reference [Radians/s] vs. Timestamp [s]"
controllerNames = ["carouselSpeedSetpoint"] # These are members in the struct, apparently
controllerNamesExt = controllerNames + genNames
controllerPlots = addPlotsToLayout(layout.addLayout(), controllerTitle, controllerNames)


# lisa gyro sensor measurements
armboneGyroTitle = "Armbone Lisa Gyro Sensor Data [Radians/s]"
armboneGyroNames = ["gp","gq","gr"] # These are members in the struct, apparently
armboneGyroNamesExt = armboneGyroNames + genNames
armboneGyroPlots = addPlotsToLayout(layout.addLayout(), armboneGyroTitle, armboneGyroNames)

# lisa acceleration sensor measurements
armboneAccelTitle = "Armbone Acceleration Sensor Data [Meters/s^2]"
armboneAccelNames = ["ax", "ay","az"] # These are members in the struct, apparently
armboneAccelNamesExt = armboneAccelNames + genNames
armboneAccelPlots = addPlotsToLayout(layout.addLayout(), armboneAccelTitle, armboneAccelNames)

# lisa mag sensor measurements
armboneMagTitle = "Armbone Magnetic Sensor Data [Gauss? Tesla?]"
armboneMagNames = ["mx","my","angle"] # These are members in the struct, apparently
armboneMagNamesExt = armboneMagNames + genNames
armboneMagPlots = addPlotsToLayout(layout.addLayout(), armboneMagTitle, armboneMagNames)

# Setup update of the plotter

# Queue for data exchange between the worker and the main thread
import Queue
q1 = Queue.Queue(maxsize = 10)
q2 = Queue.Queue(maxsize = 10)
q3 = Queue.Queue(maxsize = 10)
q4 = Queue.Queue(maxsize = 10)
q5 = Queue.Queue(maxsize = 10)
q6 = Queue.Queue(maxsize = 10)

def updatePlots():
    global q1
    global q2
    global q3
    global q4
    global q5
    global q6

    global lasPlots, lasNames \
            , carouselPlots, carouselNames \
            , controllerPlots, controllerNames \
            , armboneGyroPlots, armboneGyroNames \
            , armboneAccelPlots, armboneAccelNames\
            , armboneMagPlots, armboneMagNames

    def updateGroup(q, plots, names):
        try:
            data = q.get_nowait()
            timeStamps = data[ "ts_trigger" ]
            map(lambda name: plots[ name ].setData(timeStamps, data[ name ] ), names)
            
        except Queue.Empty:
            pass
        
    # Update plots
    updateGroup(q1, lasPlots, lasNames)
    updateGroup(q2, carouselPlots, carouselNames)
    updateGroup(q3, controllerPlots, controllerNames)
    updateGroup(q4, armboneGyroPlots, armboneGyroNames)
    updateGroup(q5, armboneAccelPlots, armboneAccelNames)
    updateGroup(q6, armboneMagPlots, armboneMagNames)

# Set up a timer to update the plots
timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 100 ) # Tick in [ms]

def signal_handler(signal, frame):
        print('Exiting because you pressed Ctrl+C!')
        sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

# ZMQ part:

# Create workers
workers = []

# NOTE: All buffer lenghts are set to correspond to last 20 sec.
# TODO: Somehow we should automate this...

workers.append(ZmqSubProtobufWorker(host + ":" + ports['lineAngleSensor2Telemetry'], LineAnglesMsg, lasNamesExt, q1, bufferSize = 20 * 100))
workers.append(ZmqSubProtobufWorker(host + ":" + ports['siemensSensorsTelemetry'], SiemensDriveStateMsg, carouselNamesExt, q2, bufferSize = 20 * 100))
workers.append(ZmqSubProtobufWorker(host + ":" + ports['controllerTelemetry'], SiemensDriveCommandMsg, controllerNamesExt, q3, bufferSize = 20 * 100))
workers.append(ZmqSubProtobufWorker(host + ":" + ports['armboneGyroTelemetry'], ImuGyroMsg, armboneGyroNamesExt, q4, bufferSize = 20 * 100))
workers.append(ZmqSubProtobufWorker(host + ":" + ports['armboneAccelTelemetry'], ImuAccelMsg, armboneAccelNamesExt, q5, bufferSize = 20 * 100))
workers.append(ZmqSubProtobufWorker(host + ":" + ports['armboneMagTelemetry'], ImuMagMsg, armboneMagNamesExt, q6, bufferSize = 20 * 100))

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
