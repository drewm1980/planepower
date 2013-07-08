#!/usr/bin/env rttlua-i

dofile("preamble.lua")

deployer:import("masterTimer")
deployer:loadComponent("masterTimer","MasterTimer")
masterTimer=deployer:getPeer("masterTimer")

dofile("load_carousel_1_hardware.lua")
dofile("configure_carousel_1_hardware.lua")

-- Master timer base clock is clock of our fastest running sensor
deployer:loadService("masterTimer", "marshalling")
masterTimer.marshalling.loadProperties("../../properties/masterTimer.cpf")
--base_hz = masterTimer.imu_target_hz

--deployer:import("measurementsRecorder")
--deployer:loadComponent("measurementsRecorder","MeasurementsRecorder")
--measurementsRecorder=deployer:getPeer("measurementsRecorder")

--deployer:import("poseFromMarkers")
--deployer:loadComponent("poseFromMarkers","PoseFromMarkers")
--poseFromMarkers=deployer:getPeer("poseFromMarkers")

----rtt.setLogLevel("Debug")
--deployer:import("simpleTrajectoryGenerator")
--deployer:loadComponent("trajectoryGenerator","SimpleTrajectoryGenerator")
--trajectoryGenerator=deployer:getPeer("trajectoryGenerator")
----rtt.setLogLevel(globalLogLevel)

--deployer:import("protobufBridge")
--deployer:loadComponent("protobuf","ProtobufBridge")
--protobuf=deployer:getPeer("protobuf")

--deployer:import("dynamicMHE")
--deployer:loadComponent("dynamicMHE","DynamicMHE")
--dynamicMHE=deployer:getPeer("dynamicMHE")

--deployer:import("dynamicMPC")
--deployer:loadComponent("dynamicMPC","DynamicMPC")
--dynamicMPC=deployer:getPeer("dynamicMPC")

dofile("postamble.lua")
