#!/usr/bin/env rttlua-i
dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property",
						"sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end
require "running_helpers"
for i,symbol in ipairs({"set_voltage",
						"get_ddelta",
						"set_controls_unitless"}) do
	_G[symbol] = running_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
--dofile("setup_carousel_1_reporters.lua")
dofile("setup_carousel_1_telemetry.lua")

masterTimer:start()

--p=encoder:getPort("data")
--set_voltage(4)
--sleep(1.1)
--set_voltage(0)

dofile("postamble.lua")
