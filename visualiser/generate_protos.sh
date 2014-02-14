#! /usr/bin/env bash

# Helpers for paths...
C1=$PLANEPOWER_ROOT/components/carousel1
MAIN=../components/main

# Generate Python protobuf bindings

protoc --python_out=. -I$C1/mcuHandler $C1/mcuHandler/McuHandlerTelemetry.proto

protoc --python_out=. -I$C1/encoder $C1/encoder/EncoderTelemetry.proto

protoc --python_out=. -I$C1/lineAngleSensor $C1/lineAngleSensor/LineAngleSensorTelemetry.proto

protoc --python_out=. -I$C1/winchControl $C1/winchControl/WinchControlTelemetry.proto

protoc --python_out=. -I$MAIN/LEDTracker $MAIN/LEDTracker/LEDTrackerTelemetry.proto

# For the MHE, this is done in a very ugly way at the moment
python -B ../rawesome_stuff/generateMhe.py ../properties
protoc --python_out=. DynamicMheTelemetry.proto
