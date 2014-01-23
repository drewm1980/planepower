#!/usr/bin/env rttlua
-- Sometimes the plane servos make an annoying noise when they get
-- stuck just slightly uncentered.  This is harmless, but you can
-- run this script to wiggle the servos just a bit to stop the noise.

dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property",
					    "sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
dofile("setup_carousel_1_reporters.lua")

masterTimer:start()

t = 0.04
A = 0.04

for i =1,3 do
	mcuHandler:setControlsUnitless(A,A,A)
	sleep(t)
	mcuHandler:setControlsUnitless(-A,-A,-A)
	sleep(t)
end
mcuHandler:setControlsUnitless(0,0,0)
