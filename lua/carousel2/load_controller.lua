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

libraryNames={"resampler",
				"gainLoader" }
classNames={"Resampler",
				"GainLoader" }
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

-- This would be a convenient place to switch between controllers:
-- It is probably best to keep the instance named "controller"
load_component("controllerTemplate","ControllerTemplate","controller")


----------------- Set Priorities and activities

-- The resampler component clocks the whole control chain;
-- The rest are successively triggered using event ports.
deployer:setActivityOnCPU("resampler", 1.0/controlFrequency, controllerPrio, scheduler,quietCore)
deployer:setActivityOnCPU("gainLoader", 0.0, humanPrio, scheduler,someNoisyCore)
deployer:setActivityOnCPU("controller", 0.0, controllerPrio, scheduler,quietCore)

---------------- Connect Components

cp = rtt.Variable("ConnPolicy")

deployer:connect("siemensSensors.data","resampler.driveState", cp)
deployer:connect("lineAngleSensor2.data","resampler.lineAngles", cp)
deployer:connect("resampler.data","controller.resampledMeasurements", cp)
deployer:connect("gainLoader.gains", "controller.gains", cp)
deployer:connect("controller.data","siemensActuators.controls",cp)

-- We need to load in at least one set of controller gains before the controller can transition to the runing state.

GAINSDIR = PLANEPOWER.."components/carousel2/gainLoader/"
--store_properties("gainLoader",GAINSDIR.."gains.cpf")
load_properties("gainLoader",GAINSDIR.."gains.cpf")
gainLoader:trigger()

--------------- Configure and start the components
for i=1,#instanceNames do
	_G[instanceNames[i]]:configure()
	_G[instanceNames[i]]:start()
end
sleep(.001)
controller:configure()

