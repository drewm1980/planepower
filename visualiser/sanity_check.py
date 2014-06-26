#!/usr/bin/env python
import sys

############### Import protobuf types #####################
paths=['siemensSensors','siemensActuators','resampler','lineAngleSensor2']
for p in paths:
    paath = '../components/carousel2/' + p + '/types'
    print "Adding path " + paath + " to the python search path..."
    sys.path.append(paath)

print "Importing protobufs..."
protobufs = ["LineAngles", "ResampledMeasurements", "SiemensDriveState", "SiemensDriveCommand",]
for p in protobufs:
    exec "from %sTelemetry_pb2 import %sMsg" % (p, p)


############### Setup zmq
import zmq
context = zmq.Context().instance()
socket = context.socket(zmq.SUB)
socket.connect('tcp://10.42.0.21:5562') # 2,3,4,5
socket.setsockopt(zmq.SUBSCRIBE, '')


print "Starting event loop..."
#for i in xrange(100):
print "Polling with 100ms timeout"
while True:
    if socket.poll(timeout=1000) != 0:
        #print "Receiving a packet!!!!"
        raw = socket.recv()
        msg = SiemensDriveStateMsg()
        msg.Clear()
        msg.ParseFromString(raw)
        print msg.winchSpeedSetpoint
        print msg.winchSpeedSmoothed
        print msg.winchEncoderPosition
        print msg.winchTorque
        print msg.winchCurrent
        print msg.carouselSpeedSetpoint
        print msg.carouselSpeedSmoothed
        print msg.carouselEncoderPosition
        print msg.carouselTorque
        print msg.carouselCurrent


# Protocol buffers are not safe.  NO type checking whatsoever at parse time.


