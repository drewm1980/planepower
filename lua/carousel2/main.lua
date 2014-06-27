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
function ramp_to(targetSpeed)
	acceleration = .1
	dt = .5 -- s
	threshold = .2 -- Rad/s
	while true do
		currentSpeed = get_carousel_speed()
		if math.abs(currentSpeed - targetSpeed) < threshold then
			print "Ramp goal achieved!"
			return
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

function step()
	--Set the parameterf of our function generator for a step response
	stepheight = 1.0 -- Rad/s
	lowtime = 4.0 -- seconds.  This is also the hightime.  Make longer than your settling time.

	type = 1 -- for square wave
	whichDrive = 1 -- for carousel
	amplitude = stepheight/2.0
	phase = 2.0*3.1415
	offset = amplitude
	period = 2.0*lowtime
	frequency = 1.0/period

	ramp_to(offset)
	sleep(1)
	set_property("functionGenerator","type",type)
	set_property("functionGenerator","amplitude",amplitude)
	set_property("functionGenerator","phase",phase)
	set_property("functionGenerator","offset",offset)
	set_property("functionGenerator","frequency",frequency)
	set_property("functionGenerator","whichDrive",whichDrive)
	functionGenerator:start()
end

function stopFunctionGenerator()
	functionGenerator:stop()
	ramp_to(0)
end
----------------- THE EXPERIMENT!!!!!!! -------------
function run()
	speedOffset = 2
	ramp_to(speedOffset)
	sleep(10)
	ramp_to(0.0)
end

dofile("../shared/postamble.lua")
