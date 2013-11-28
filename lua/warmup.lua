#!/usr/bin/env rttlua
-- Sometimes the plane servos make an annoying noise when they get
-- stuck just slightly uncentered.  This is harmless, but you can
-- run this script to wiggle the servos just a bit to stop the noise.

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
						"get_ddelta"}) do
        _G[symbol] = running_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
--dofile("setup_carousel_1_reporters.lua")

masterTimer:start()

pi = 3.1415
thresh_warm = 52.0  -- rpm
thresh_stopped = 1.0 

print("Spinning up carousel...")
set_voltage(5)
repeat 
	os.execute("sleep 0.2")
	ddelta = get_ddelta()
	print("Speed is ".. ddelta .. " RPM")
	--io.stdout:flush()
	--if io.read(0) then enterPressed=true; break end
	--if io.read(0) then io.read(); enterPressed=true; break end
until ddelta >= thresh_warm
if enterPressed then 
	print("Enter Pressed!")
	enterPressed=false 
else 
	print("Carousel is warm; spinning down...")
end

set_voltage(0)
repeat
	os.execute("sleep 0.1")
	ddelta = get_ddelta()
	print("Speed is " .. ddelta .. " RPM")
	io.stdout:flush()
	--if io.read(0) then io.read(); enterPressed=true; break end
until ddelta < thresh_stopped
if enterPressed then 
	print("Enter Pressed!"); 
	enterPressed=false 
else
	print("Done spinning down!")
end


