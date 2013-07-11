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
deployer:loadComponent("soemMaster","soem_master::SoemMasterComponent")
soemMaster=deployer:getPeer("soemMaster")
set_property("soemMaster","ifname","eth1")
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

set_property("mcuHandler","Ts",1.0/base_hz)
set_property("mcuHandler","rtMode",true)

set_property("encoder","encoderPort",0)
set_property("encoder","Ts",0.001)

-- LED tracker - hangs until frame arrival, does processing, and re-triggers itself.
set_property("LEDTracker","useExternalTrigger",true)
set_property("LEDTracker","sigma_marker",20)

-- Load and set up a reporter for each sensor type
reporterBaseNames={"imu","camera","encoder","lineAngle"}
reportedComponentNames={"mcuHandler","LEDTracker","encoder","lineAngleSensor"}
reporterNames={}
reporterFileNames={}
for i=1,#reporterBaseNames do 
	reporterNames[i]=reporterBaseNames[i].."Reporter"
	reporterFileNames[i]=reporterBaseNames[i].."Data.nc"
end
rp = rtt.Variable("ConnPolicy")
rp.type = 2
rp.size = 4096
for i,reporterName in pairs(reporterNames) do
	load_reporter(reporterName)
	set_property(reporterName,"ReportFile",reporterFileNames[i])
	set_property(reporterName,"ReportPolicy",rp)
	set_property(reporterName,"ReportOnlyNewData",false)
	deployer:connectPeers(reporterName,reportedComponentNames[i])
	deployer:setActivity(reporterName,0.0,reporterPrio,ORO_SCHED_RT)
end


