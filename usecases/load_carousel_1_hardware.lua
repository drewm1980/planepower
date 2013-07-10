#!/usr/bin/env rttlua-i
require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_reporter",
						"load_properties",
						"get_property",
						"set_property"}) do
	_G[symbol] = deployment_helpers[symbol]
end

dofile("preamble.lua")

-- Fully start up soem hardware before anything else
deployer:import("soem_master")
deployer:import("soem_ebox")
load_component("soemMaster","soem_master::SoemMasterComponent","soemMaster")
--deployer:loadComponent("soemMaster","soem_master::SoemMasterComponent")
--soemMaster=deployer:getPeer("soemMaster")
set_property("soemMaster","ifname","eth1")
--soemMaster:getProperty("ifname"):set("eth1")
soemPrio=99
deployer:setActivity("soemMaster", 0.001, soemPrio, ORO_SCHED_RT)

-- Load up the rest of the Carousel 1 hardware that interfaces with sensors and hardware
libraryNames={"mcuHandler","encoder","cameraTrigger","lineAngleSensor","LEDTracker"}
classNames={  "McuHandler","Encoder","CameraTrigger","LineAngleSensor","LEDTracker"}
instanceNames=libraryNames

for i=1,#libraryNames do
	load_component(libraryNames[i],classNames[i],instanceNames[i])
end

-- Configure the hardware components
load_properties("mcuHandler",PROPERTIES.."tcp.cpf")

mcuHandler.Ts = 1.0 / base_hz
mcuHandler.rtMode = true


-- Encoder
encoder.encoderPort = 0;
-- Same as for SOEM
encoder.Ts = 0.001;

-- LED tracker - hangs until frame arrival, does processing, and re-triggers itself.
LEDTracker.useExternalTrigger = true;
LEDTracker.sigma_marker = 20;


-- Load a reporter for each sensor type
reporterBaseNames={"imu","camera","encoder","lineAngle"}
reporterNames={}
for i=1,#reporterBaseNames do 
	reporterNames[i]=reporterBaseNames[i].."Reporter"
end
for key,reporterName in pairs(reporterNames) do
	load_reporter(reporterName)
end


