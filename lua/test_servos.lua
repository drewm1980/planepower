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
for i,symbol in ipairs({"set_controls_unitless"}) do
	_G[symbol] = running_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
--dofile("setup_carousel_1_reporters.lua")

masterTimer:start()

t = 0.3
A = 0.75
--A = 1.0 

trials = 3

-- Check full scale movement, and sign
print "Checking full scale movement of all servos..."
t2 = 2
set_controls_unitless(1,1,1)
sleep(t2)
set_controls_unitless(-1,-1,-1)
sleep(t2)
set_controls_unitless(0,0,0)
sleep(t2)

-- Check servos individually
print "Checking servos individually so you can observe the sign..."
sleep(t)
for i =1,trials do
	set_controls_unitless(A,0,0)
	sleep(t)
	set_controls_unitless(-A,0,0)
	sleep(t)
end
set_controls_unitless(0,0,0)
sleep(t)
for i =1,trials do
	set_controls_unitless(0,A,0)
	sleep(t)
	set_controls_unitless(0,-A,0)
	sleep(t)
end
set_controls_unitless(0,0,0)
for i =1,trials do
	set_controls_unitless(0,0,A)
	sleep(t)
	set_controls_unitless(0,0,-A)
	sleep(t)
end
set_controls_unitless(0,0,0)
sleep(t)
print "done"

