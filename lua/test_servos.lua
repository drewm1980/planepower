#!/usr/bin/env rttlua-i
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

t = 0.4
A = 0.5
--A = 1.0 

for i =1,5 do
	mcuHandler:setControlsUnitless(A,A,A)
	os.execute("sleep " .. t)
	mcuHandler:setControlsUnitless(-A,-A,-A)
	os.execute("sleep " .. t)
end
mcuHandler:setControlsUnitless(0,0,0)

--dofile("postamble.lua")
