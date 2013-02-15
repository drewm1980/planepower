#!/usr/bin/env bash

# Upgrade the components in lowlevel
# Building: 
building="\
masterTimer \
planeSimulator \
planeSimulatorRframe \
imuBuffer \
cameraSimulator \
imuSimulator \
LEDTrackerSimulator \
"
converting="\
protobufBridge \
planeSimulatorDirectControls \
planeSimulatorRframeDirectControls \
"
unknown="\
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

for x in $unknown; do ../upgrade_component_to_tup.sh $x; done

