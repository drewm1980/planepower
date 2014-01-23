#!/usr/bin/env rttlua
-- A script for Kurt to gather open-loop
-- measurements with random servos

dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property",
					    "set_up_reporters"}) do
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

-- Load up a component to set controls that are synchronized with
-- the camera clock
load_component("controlInjector","ControlInjector","controlInjector",cp)
deployer:connect("masterTimer.cameraClock","controlInjector.trigger",cp)
deployer:connect("controlInjector.portControls","mcuHandler.controls",cp)

masterTimer:start()
controlInjector:start()

function random_control(min,max)
	r = math.random(0,1)
	return r*(max-min) + min
end

--warmup()
dofile("setup_carousel_1_reporters.lua")
set_up_reporters({"controls"},{"controlInjector"})

os.execute("sleep ".. 3)

t = 0.08 -- seconds
A = 0.25 
print "Starting to gather data"
for i =1,3750 do
	r_aileron = random_control(-A,A)
	l_aileron = r_aileron
	elevator = random_control(-A,A)
	mcuHandler:setControlsUnitless(r_aileron,l_aileron,elevator)
	os.execute("sleep " .. t)
end
mcuHandler:setControlsUnitless(0,0,0)
print "finished gathering data. Slowing down"
os.execute("sleep ".. 3)

slowdown()
