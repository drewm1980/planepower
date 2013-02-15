#!/usr/bin/env bash

# Upgrade the components in lowlevel
# Building: 
c=""
# Not building yet:

#masterTimer
#planeSimulator
#planeSimulatorDirectControls
#planeSimulatorRframe
#planeSimulatorRframeDirectControls
#protobufBridge
#imuBuffer
#cameraSimulator
#imuSimulator
#LEDTrackerSimulator
#measurementsPlayback
#measurementsRecorder
#dynamicMHE
#dynamicMPC

#cameraTrigger
#LEDTracker

#calibrationCamera
#controlDerivativeHandler
#controlsPlayback
#lqrController
#lqrController_control_derivatives
#manualController
#playControls
#poseFromMarkers
#simpleTrajectoryGenerator

for x in $c; do ../upgrade_component_to_tup.sh $x; done

