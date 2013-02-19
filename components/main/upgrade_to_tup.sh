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
measurementsRecorder \
cameraTrigger \
controlDerivativeHandler \
lqrController \
manualController \
playControls \
simpleTrajectoryGenerator\
controlsPlayback \
measurementsPlayback \
poseFromMarkers \
protobufBridge \
planeSimulatorDirectControls \
planeSimulatorRframeDirectControls \
"
converting="\
lqrController_control_derivatives \
LEDTracker \
dynamicMHE \
dynamicMPC \
calibrationCamera \
"
unknown="\
"
for x in $building $converting; do ../upgrade_component_to_tup.sh $x; done

git rm make_all.sh
