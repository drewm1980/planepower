#!/usr/bin/env bash

rosmake --build-everything calculateCameraParameters calibrationCamera cameraCalibration cameraSimulator cameraTrigger controlDerivativeHandler controlsPlayback dynamicMHE dynamicMPC imuBuffer imuSimulator LEDTracker LEDTrackerSimulator lqrController lqrController_control_derivatives manualController masterTimer measurementsPlayback measurementsRecorder planeSimulator planeSimulatorDirectControls planeSimulatorRframe planeSimulatorRframeDirectControls playControls poseFromMarkers protobufBridge simpleTrajectoryGenerator winchControl

echo "There are `find . -name manifest.xml | wc -l` manifest.xml's"
echo "There are `find . -name *-gnulinux.so | wc -l` things that look like build components"
