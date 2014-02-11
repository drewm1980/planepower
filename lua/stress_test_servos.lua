#!/usr/bin/env rttlua
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
for i,symbol in ipairs({"set_controls_unitless"}) do
	_G[symbol] = running_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
--dofile("setup_carousel_1_reporters.lua")

masterTimer:start()

t = 0.2
A = 0.7
--A = 1.0 

--trials = 100

print "Stress testing all servos..."
--for i=1,trials do
i = 1
while true do
	--print("rep ".. i .. " of " .. trials .. "..")
	print("rep "..i)
	set_controls_unitless(1,1,1)
	sleep(t)
	set_controls_unitless(-1,-1,-1)
	sleep(t)
	i = i+1
end
set_controls_unitless(0,0,0)
print "done!"

