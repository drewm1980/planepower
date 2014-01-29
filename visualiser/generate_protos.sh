#! /usr/bin/env bash

# Helpers for paths...
C1=$PLANEPOWER_ROOT/components/carousel1
MAIN=../components/main

echo $C1
echo $MAIN

# Specify proto files

protoc --python_out=. -I$C1/mcuHandler $C1/mcuHandler/McuHandlerTelemetry.proto

protoc --python_out=. -I$C1/encoder $C1/encoder/EncoderTelemetry.proto

protoc --python_out=. -I$C1/lineAngleSensor $C1/lineAngleSensor/LineAngleSensorTelemetry.proto

protoc --python_out=. -I$C1/winchControl $C1/winchControl/WinchControlTelemetry.proto

protoc --python_out=. -I$MAIN/LEDTracker $MAIN/LEDTracker/LEDTrackerTelemetry.proto
