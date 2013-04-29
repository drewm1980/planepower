dofile("preamble.lua")

deployer:import("masterTimer")
deployer:loadComponent("masterTimer","OCL::MasterTimer")
masterTimer=deployer:getPeer("masterTimer")

--deployer:import("encoderLatch")
--deployer:loadComponent("EncoderLatch","OCL::EncoderLatch")
--EncoderLatch=deployer:getPeer("EncoderLatch")

--deployer:import("LEDTrackerSimulator")
--deployer:loadComponent("LEDTrackerSimulator","OCL::LEDTrackerSimulator")
--LEDTrackerSimulator=deployer:getPeer("LEDTrackerSimulator")

--deployer:import("imuBuffer")
--deployer:loadComponent("imuBuffer","OCL::ImuBuffer")
--imuBuffer=deployer:getPeer("imuBuffer")

--deployer:import("measurementsRecorder")
--deployer:loadComponent("measurementsRecorder","OCL::MeasurementsRecorder")
--measurementsRecorder=deployer:getPeer("measurementsRecorder")

--deployer:import("planeSimulatorRframe")
--deployer:loadComponent("sim","OCL::planeSimulatorRframe")
--sim=deployer:getPeer("sim")

--deployer:import("cameraSimulator")
--deployer:loadComponent("cameraSimulator","OCL::CameraSimulator")
--cameraSimulator=deployer:getPeer("cameraSimulator")

--deployer:import("poseFromMarkers")
--deployer:loadComponent("poseFromMarkers","OCL::PoseFromMarkers")
--poseFromMarkers=deployer:getPeer("poseFromMarkers")

----rtt.setLogLevel("Debug")
--deployer:import("simpleTrajectoryGenerator")
--deployer:loadComponent("trajectoryGenerator","OCL::SimpleTrajectoryGenerator")
--trajectoryGenerator=deployer:getPeer("trajectoryGenerator")
----rtt.setLogLevel(globalLogLevel)

--deployer:import("protobufBridge")
--deployer:loadComponent("protobuf","OCL::ProtobufBridge")
--protobuf=deployer:getPeer("protobuf")

--deployer:import("dynamicMHE")
--deployer:loadComponent("dynamicMHE","DynamicMHE")
--dynamicMHE=deployer:getPeer("dynamicMHE")

--deployer:import("dynamicMPC")
--deployer:loadComponent("dynamicMPC","DynamicMPC")
--dynamicMPC=deployer:getPeer("dynamicMPC")

dofile("postamble.lua")
