#!/usr/bin/env rttlua-i
dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property"}) do
	_G[symbol] = deployment_helpers[symbol]
end
require "running_helpers"
for i,symbol in ipairs({ "set_voltage"}) do
	_G[symbol] = running_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
dofile("setup_carousel_1_reporters.lua")

masterTimer:start()

--set_voltage(4)
--os.execute("sleep 5")
--set_voltage(0)

dofile("postamble.lua")
