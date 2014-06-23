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

rtt.logl("Info", "Loading PLANEPOWER components...")

useSimulators = true 
-- Set whether we will use the real hardware, or the *Simulator components
if useSimulators then
	normalPrio = 20 -- not sure if these go into prio or rtprio...
	controllerPrio = normalPrio
	sensorPrio = normalPrio
	reporterPrio = normalPrio
	humanPrio = normalPrio
else
	-- You need to be superuser for this, or you will be flodded
	-- with warnings, and the priorities will not be realtime!
	controllerPrio = 98
	sensorPrio = 97
	reporterPrio = 50
	humanPrio = 60
end

dofile("load_hardware.lua")
dofile("load_controller.lua")

--dofile("setup_reporters.lua")
--dofile("setup_telemetry.lua")

--masterTimer:start()

dofile("../shared/postamble.lua")
