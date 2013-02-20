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
simpleTrajectoryGenerator \
controlsPlayback \
measurementsPlayback \
poseFromMarkers \
protobufBridge \
planeSimulatorDirectControls \
planeSimulatorRframeDirectControls \
cameraTrigger \
"
converting="\
LEDTracker \
calibrationCamera \
dynamicMHE \
dynamicMPC \
lqrController_control_derivatives \
calculateCameraParameters \
winchControl \
"

# calibrationCamera depends on ROS to talk to cameras
# dynamicMHE,MPC need to be ported manually from CMake

for x in $converting $building; do ../upgrade_component_to_tup.sh $x; done

git rm make_all.sh


