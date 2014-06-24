#!/usr/bin/env rttlua-i

package.path = package.path .. ';../shared/?.lua'

dofile("../shared/preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property",
						"sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end

useSimulators = true 
-- Set whether we will use the real hardware, or the *Simulator components
if useSimulators then
	-- This mode is good for just checking if you got (most of)
	-- your components hooked up right
	quietCore = 0 -- So that we can test software on singlecore devices
	someNoisyCore = 0
	normalPrio = 0 -- not sure if these go into prio or rtprio...
	scheduler = ORO_SCHED_OTHER
	controllerPrio = normalPrio
	sensorPrio = normalPrio
	reporterPrio = normalPrio
	humanPrio = normalPrio
	telemetryPrio = normalPrio
else
	-- You need to be superuser for this, or you will be flodded
	-- with warnings, and the priorities will not be realtime!
	quietCore = 6 -- For the carousel
	someNoisyCore = 1
	scheduler = ORO_SCHED_RT
	controllerPrio = 97
	sensorPrio = 96
	reporterPrio = 50
	humanPrio = 60
	telemetryPrio = 30
end

rtt.logl("Info", "Loading HIGHWIND hardware related components...")
dofile("load_hardware.lua")

rtt.logl("Info", "Loading HIGHWIND controller related components...")
controlFrequency = 50.0 -- Hz
dofile("load_controller.lua")

rtt.logl("Info", "Loading HIGHWIND logging related components...")
dofile("load_reporters.lua")

rtt.logl("Info", "Loading HIGHWIND telemetry related components...")
telemetryFrequency = 50.0 --Hz
dofile("load_telemetry.lua")

-- controller:start()

dofile("../shared/postamble.lua")
