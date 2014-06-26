#!/usr/bin/env rttlua-i

package.path = package.path .. ';../shared/?.lua'

dofile("../shared/preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property",
						"sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end

useSimulators = false 
-- Set whether we will use the real hardware, or the *Simulator components
if useSimulators then
	-- This mode is good for just checking if you got (most of)
	-- your components hooked up right
	quietCore = 0 -- So that we can test software on singlecore devices
	someNoisyCore = 0
	normalPrio = 0 -- not sure if these go into prio or rtprio...
	scheduler = ORO_SCHED_OTHER
	controllerPrio = normalPrio
	sensorPrio = normalPrio
	reporterPrio = normalPrio
	humanPrio = normalPrio
	telemetryPrio = normalPrio
else
	-- You need to be superuser for this, or you will be flodded
	-- with warnings, and the priorities will not be realtime!
	quietCore = 6 -- For the carousel
	someNoisyCore = 1
	scheduler = ORO_SCHED_RT
	controllerPrio = 97
	sensorPrio = 96
	reporterPrio = 50
	humanPrio = 60 -- For human interaction, i.e. gain loading
	telemetryPrio = 30
end

rtt.logl("Info", "Loading HIGHWIND hardware related components...")
dofile("load_hardware.lua")

measuringStepResponses=true
if measuringStepResponses then
	rtt.logl("Info", "Loading function generator component...")
	dofile("load_function_generator.lua")
else
	controlFrequency = 50.0 -- Hz
	rtt.logl("Info", "Loading HIGHWIND controller related components...")
	dofile("load_controller.lua")
end

rtt.logl("Info", "Loading HIGHWIND logging related components...")
dofile("load_reporters.lua")

rtt.logl("Info", "Loading HIGHWIND telemetry related components...")
telemetryFrequency = 50.0 --Hz
dofile("load_telemetry.lua")

if useSimulators then
	controller:start()
end

function set_carousel_speed(speed)
	siemensActuators:stop()
	siemensActuators:getOperation('setCarouselSpeed')(speed)
end
function get_carousel_speed()
	return siemensSensors:provides("data"):last()['carouselSpeedSmoothed']
end

require "math"

softlimit = 3.1415 -- Rad/s

-- ALWAYS check the return value of this!
function ramp_to(targetSpeed)
	acceleration = .1
	dt = .2 -- s
	threshold = .004 -- Rad/s
	if (math.abs(targetSpeed) > softlimit) then
		print "Requested speed is outside the soft limit!"
		return 0
	end

	while true do
		currentSpeed = get_carousel_speed()
		if math.abs(currentSpeed - targetSpeed) < threshold then
			print "Ramp goal achieved!"
			return 1
		end
		if currentSpeed > targetSpeed then
			print "Ramping down..."
			nextspeed = math.max(targetSpeed, currentSpeed - dt*acceleration)
		else
			print "Ramping up..."
			nextspeed = math.min(targetSpeed, currentSpeed + dt*acceleration)
		end
		print ("Target Speed: "..tostring(targetSpeed).." Current Speed: "..tostring(currentSpeed).." Next Speed: "..tostring(nextspeed))
		set_carousel_speed(nextspeed)
		sleep(dt)
	end
end

----------------- THE EXPERIMENT!!!!!!! -------------
function run()
	speedOffset = softlimit/2.0
	if (ramp_to(speedOffset)) then
		ramp_to(0.0)
		return 0
	end
	sleep(10)
	ramp_to(0.0)
	return 1
end

dofile("../shared/postamble.lua")
