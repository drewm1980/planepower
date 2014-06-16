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

dofile("load_hardware.lua")
--dofile("setup_reporters.lua")
--dofile("setup_telemetry.lua")

--masterTimer:start()

dofile("../shared/postamble.lua")
