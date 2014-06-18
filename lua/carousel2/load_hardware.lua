#!/usr/bin/env rttlua-i
require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_reporter",
						"load_properties",
						"get_property",
						"set_property",
						"sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end

PLANEPOWER="../../"
PROPERTIES=PLANEPOWER.."properties/"

controllerPrio = 98
sensorPrio = 97
reporterPrio = 50

--load_component("masterTimer","MasterTimer","masterTimer")
--load_properties("masterTimer",PROPERTIES.."masterTimer.cpf")
--base_hz = get_property("masterTimer","imu_target_hz")

libraryNames={"siemensActuators",
				"siemensSensors",
				"lineAngleSensor2"}
--classNames = libraryNames:gsub("(%l)(%w*)", function(a,b) return string.upper(a)..b end)
classNames={"SiemensActuators",
				"SiemensSensors",
				"LineAngleSensor2"}
instanceNames=libraryNames

-- Load up the Carousel 2 hardware that interfaces with sensors and hardware
for i=1,#libraryNames do
	load_component(libraryNames[i],classNames[i],instanceNames[i])
end

-- Configure the hardware components
--load_properties("mcuHandler",PROPERTIES.."tcp.cpf") -- Throwing "TinyDemarshaller" type warnings

----------------- Set Priorities and activities

deployer:setActivityOnCPU("siemensDrives", 0.0, sensorPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("lineAngleSensor2", 0.0, sensorPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("siemensActuators", 0.0, sensorPrio, ORO_SCHED_RT,6)
--deployer:setActivityOnCPU("downsampler", 10, controllerPrio, ORO_SCHED_RT,2)

---------------- Connect Components

cp = rtt.Variable("ConnPolicy")

--deployer:connect("masterTimer.imuClock","mcuHandler.trigger", cp)
--deployer:connect("masterTimer.cameraClock", "cameraTrigger.Trigger", cp)

--------------- Configure and start the components

for i=1,#instanceNames do
	_G[instanceNames[i]]:configure()
	_G[instanceNames[i]]:start()
end


