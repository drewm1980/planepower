#!/usr/bin/env bash

# Upgrade the components in lowlevel
# Building: 
upgrade="\
masterTimer \
planeSimulator \
planeSimulatorDirectControls \
planeSimulatorRframe \
planeSimulatorRframeDirectControls \
protobufBridge \
imuBuffer \
cameraSimulator \
imuSimulator \
LEDTrackerSimulator \
"
noupgrade="\
measurementsPlayback \
measurementsRecorder \
cameraTrigger \
LEDTracker \
dynamicMHE \
dynamicMPC \
calibrationCamera \
controlDerivativeHandler \
controlsPlayback \
lqrController \
lqrController_control_derivatives \
manualController \
playControls \
poseFromMarkers \
simpleTrajectoryGenerator"

for x in $upgrade; do ../upgrade_component_to_tup.sh $x; done

