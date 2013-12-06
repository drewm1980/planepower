#!/usr/bin/env rttlua
-- A script for Kurt to gather open-loop
-- measurements with random servos

dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property"}) do
	_G[symbol] = deployment_helpers[symbol]
end

require "running_helpers"
for i,symbol in ipairs({"set_voltage",
						"get_ddelta",
					    "warmup",
						"slowdown"}) do
        _G[symbol] = running_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
dofile("setup_carousel_1_reporters.lua")

masterTimer:start()

function random_control(min,max)
	r = math.random(0,1)
	return r*(max-min) + min
end

--warmup()

os.execute("sleep ".. 10)

t = 0.1 -- seconds
A = .25 

for i =1,3 do
	r_aileron = random_control(-A,A)
	l_aileron = r_aileron
	elevator = random_control(-A,A)
	mcuHandler:setControlsUnitless(r_aileron,l_aileron,elevator)
	os.execute("sleep " .. t)
end
mcuHandler:setControlsUnitless(0,0,0)

--slowdown()
