#!/usr/bin/env rttlua
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

t = 0.3
A = 0.5
--A = 1.0 

trials = 3

-- Check full scale movement, and sign
t2 = 2
mcuHandler:setControlsUnitless(1,1,1)
os.execute("sleep " .. t2)
mcuHandler:setControlsUnitless(-1,-1,-1)
os.execute("sleep " .. t2)
mcuHandler:setControlsUnitless(0,0,0)
os.execute("sleep " .. t2)

-- Check servos individually
for i =1,trials do
	mcuHandler:setControlsUnitless(A,0,0)
	os.execute("sleep " .. t)
	mcuHandler:setControlsUnitless(-A,0,0)
	os.execute("sleep " .. t)
end
mcuHandler:setControlsUnitless(0,0,0)
os.execute("sleep " .. t)
for i =1,trials do
	mcuHandler:setControlsUnitless(0,A,0)
	os.execute("sleep " .. t)
	mcuHandler:setControlsUnitless(0,-A,0)
	os.execute("sleep " .. t)
end
mcuHandler:setControlsUnitless(0,0,0)
os.execute("sleep " .. t)
for i =1,trials do
	mcuHandler:setControlsUnitless(0,0,A)
	os.execute("sleep " .. t)
	mcuHandler:setControlsUnitless(0,0,-A)
	os.execute("sleep " .. t)
end
mcuHandler:setControlsUnitless(0,0,0)
os.execute("sleep " .. t)

