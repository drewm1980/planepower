#!/usr/bin/env rttlua-i

local P = {}
running_helpers = P

-- These lua functions make writing running scripts easier
function P.set_voltage(voltage)
	voltageController:setVoltage(0,voltage)
end

-- Return carousel arm rotation in rpm
function P.get_ddelta()
	return encoder:provides("data"):last()["omega_filt_rpm"]
end

function P.warmup()
	print("Spinning up carousel...")
	thresh_warm = 60.0  -- rpm
	set_voltage(10)
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
end

function P.slowdown()
	set_voltage(0)
	thresh_stopped = 1.0 
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
end

return running_helpers

