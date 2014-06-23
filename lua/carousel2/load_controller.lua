#!/usr/bin/env rttlua-i
require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_reporter",
						"load_properties",
						"store_properties",
						"get_property",
						"set_property",
						"sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end

PLANEPOWER="../../"

controllerPrio = 98
sensorPrio = 97
reporterPrio = 50

--load_component("masterTimer","MasterTimer","masterTimer")
--load_properties("masterTimer",PROPERTIES.."masterTimer.cpf")
--base_hz = get_property("masterTimer","imu_target_hz")

libraryNames={"resampler",
				"gainLoader",
				"controllerTemplate"}
classNames={"Resampler",
				"GainLoader",
				"ControllerTemplate"}
function deepcopy(liist)
	newlist = {}
	for i,symbol in ipairs(liist) do
		newlist[i] = symbol
	end
	return newlist
end
instanceNames=deepcopy(libraryNames)

-- Load in all of our signal processing
-- and controller code.
for i=1,#libraryNames do
	load_component(libraryNames[i],classNames[i],instanceNames[i])
end

----------------- Set Priorities and activities

deployer:setActivityOnCPU("resampler", 0.0, controllerPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("gainLoader", 0.0, humanPrio, ORO_SCHED_RT,6)
deployer:setActivityOnCPU("controllerTemplate", 0.0, controllerPrio, ORO_SCHED_RT,6)

---------------- Connect Components

cp = rtt.Variable("ConnPolicy")

deployer:connect("siemensSensors.data","resampler.driveState", cp)
deployer:connect("lineAngleSensor2.data","resampler.lineAngles", cp)
deployer:connect("resampler.data","controllerTemplate.resampledMeasurements", cp)
deployer:connect("gainLoader.cpp", "controllerTemplate.gains", cp)

-- We need to load in at least one set of controller gains before the controller can transition to the runing state.

GAINSDIR = PLANEPOWER.."components/carousel2/gainLoader/"
--store_properties("gainLoader",GAINSDIR.."gains.cpf")
load_properties("gainLoader",GAINSDIR.."gains.cpf")

--------------- Configure and start the components

for i=1,#instanceNames do
	_G[instanceNames[i]]:configure()
	_G[instanceNames[i]]:start()
end


