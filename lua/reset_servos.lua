#!/usr/bin/env rttlua
-- If for some reason the servos are left in an uncentered state,
-- you can run this script to center conveniently.
dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property"}) do
	_G[symbol] = deployment_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
dofile("setup_carousel_1_reporters.lua")

masterTimer:start()

mcuHandler:setControlsUnitless(0,0,0)
