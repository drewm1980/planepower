#!/usr/bin/env rttlua-i

dofile("preamble.lua")
dofile("load_soem.lua") -- note, still figuring out how to set Properties...

deployer:import("mcuHandler")
deployer:loadComponent("mcuHandler","McuHandler")
mcuHandler=deployer:getPeer("mcuHandler")

deployer:import("encoder")
deployer:loadComponent("encoder", "Encoder")
encoder=deployer:getPeer("encoder")

deployer:import("cameraTrigger")
deployer:loadComponent("cameraTrigger","CameraTrigger")
cameraTrigger=deployer:getPeer("cameraTrigger")

deployer:import("lineAngleSensor")
deployer:loadComponent("lineAngleSensor","LineAngleSensor")
lineAngleSensor=deployer:getPeer("lineAngleSensor")

deployer:import("LEDTracker")
deployer:loadComponent("LEDTracker","LEDTracker")
LEDTracker=deployer:getPeer("LEDTracker")

-- Reporters
deployer:loadComponent("imuReporter", "OCL::NetcdfReporting")
imuReporter=deployer:getPeer("imuReporter")
deployer:loadComponent("cameraReporter", "OCL::NetcdfReporting")
cameraReporter=deployer:getPeer("cameraReporter")
deployer:loadComponent("encoderReporter", "OCL::NetcdfReporting")
encoderReporter=deployer:getPeer("encoderReporter")
deployer:loadComponent("lineAngleReporter", "OCL::NetcdfReporting")
lineAngleReporter=deployer:getPeer("lineAngleReporter")

