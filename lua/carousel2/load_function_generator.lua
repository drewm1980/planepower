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

libraryNames={"functionGenerator"}
classNames={"FunctionGenerator"}
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
updateFreqency = 40
deployer:setActivityOnCPU("functionGenerator", 1.0/updateFrequency, controllerPrio, scheduler,quietCore)

cp = rtt.Variable("ConnPolicy")
deployer:connect("functionGenerator.data","siemensActuators.controls",cp)

-- Set the parameterf of our function generator for a step response
stepheight = 1.0 -- Rad/s
lowtime = 3.0 -- seconds.  This is also the hightime.  Make longer than your settling time.

amplitude = stepheight/2.0
phase = 2.0*3.1415
offset = amplitude
period = 2.0*lowtime
frequency = 1.0/period

--------------- Configure and start the components
functionGenerator:configure()

--------------- Set the parameters of our fuction generator
set_property(functionGenerator,"amplitude",amplitude)
set_property(functionGenerator,"phase",phase)
set_property(functionGenerator,"offset",offset)
set_property(functionGenerator,"frequency",frequency)

-- You can manually run this when you are ready!!
--functionGenerator:start()

