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

libraryNames={"siemensActuators",
				"siemensSensors",
				"lineAngleSensor2",
				"armboneLisaSensors"}
classNames={"SiemensActuators",
				"SiemensSensors",
				"LineAngleSensor2",
				"ArmboneLisaSensors"}
function deepcopy(liist)
	newlist = {}
	for i,symbol in ipairs(liist) do
		newlist[i] = symbol
	end
	return newlist
end
instanceNames=deepcopy(libraryNames)

function appendSimulator(listOfNames)
	for i,symbol in ipairs(listOfNames) do
		listOfNames[i] = listOfNames[i].."Simulator" 
	end
end
if useSimulators then 
	appendSimulator(libraryNames)
	appendSimulator(classNames)
	-- instance Names stay the same for compatibility with everything else!
end

-- Load up the Carousel 2 hardware that interfaces with sensors and hardware
for i=1,#libraryNames do
	load_component(libraryNames[i],classNames[i],instanceNames[i])
end

----------------- Set Priorities and activities

-- All of these components trigger themselves once started the first time.
deployer:setActivityOnCPU("siemensSensors", 0.0, sensorPrio, scheduler,quietCore)
deployer:setActivityOnCPU("lineAngleSensor2", 0.0, sensorPrio, scheduler,quietCore)
deployer:setActivityOnCPU("armboneLisaSensors", 0.0, sensorPrio, scheduler,quietCore)
deployer:setActivityOnCPU("siemensActuators", 0.0, sensorPrio, scheduler,quietCore)

--------------- Configure and start the components

for i=1,#instanceNames do
	_G[instanceNames[i]]:configure()
	_G[instanceNames[i]]:start()
end


