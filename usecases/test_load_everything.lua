require("rttlib")
rttlib.color=true
tc = rtt.getTC()
deployer = tc:getPeer("Deployer")

-- Load libraries and instantiate components

deployer:import("ocl")

deployer:import("masterTimer")
deployer:loadComponent("masterTimer","OCL::MasterTimer")

deployer:import("encoderLatch")
deployer:loadComponent("EncoderLatch","OCL::EncoderLatch")

deployer:import("LEDTrackerSimulator")
deployer:loadComponent("LEDTrackerSimulator","OCL::LEDTrackerSimulator")

deployer:import("imuBuffer")
deployer:loadComponent("imuBuffer","OCL::ImuBuffer")

deployer:import("measurementsRecorder")
deployer:loadComponent("measurementsRecorder","OCL::MeasurementsRecorder")

deployer:import("planeSimulatorRframe")
deployer:loadComponent("sim","OCL::planeSimulatorRframe")

deployer:import("cameraSimulator")
deployer:loadComponent("cameraSimulator","OCL::CameraSimulator")

deployer:import("poseFromMarkers")
deployer:loadComponent("poseFromMarkers","OCL::PoseFromMarkers")

deployer:import("simpleTrajectoryGenerator")
deployer:loadComponent("trajectoryGenerator","SimpleTrajectoryGenerator")

deployer:import("protobufBridge")
deployer:loadComponent("protobuf","OCL::ProtobufBridge")

--deployer:import("dynamicMHE")
--deployer:loadComponent("dynamicMHE","DynamicMHE")

--deployer:import("dynamicMPC")
--deployer:loadComponent("dynamicMPC","DynamicMPC")


-- Niceties for when we run in interactive mode
rttlib.info()
rttlib.stat()
