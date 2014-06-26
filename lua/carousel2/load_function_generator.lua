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

load_component("functionGenerator","FunctionGenerator","functionGenerator")

----------------- Set Priorities and activities
updateFrequency = 40
deployer:setActivityOnCPU("functionGenerator", 1.0/updateFrequency, controllerPrio, scheduler,quietCore)

cp = rtt.Variable("ConnPolicy")
deployer:connect("functionGenerator.data","siemensActuators.controls",cp)

-- Set the parameterf of our function generator for a step response
stepheight = 4.0 -- Rad/s
lowtime = 4.0 -- seconds.  This is also the hightime.  Make longer than your settling time.

type = 1 -- for square wave
whichDrive = 1 -- for carousel
amplitude = stepheight/2.0
phase = 2.0*3.1415
offset = amplitude
period = 2.0*lowtime
frequency = 1.0/period

--------------- Configure and start the components
functionGenerator:configure()

--------------- Set the parameters of our fuction generator
set_property("functionGenerator","type",type)
set_property("functionGenerator","amplitude",amplitude)
set_property("functionGenerator","phase",phase)
set_property("functionGenerator","offset",offset)
set_property("functionGenerator","frequency",frequency)
set_property("functionGenerator","whichDrive",whichDrive)

-- You can manually run this when you are ready!!
--functionGenerator:start()

