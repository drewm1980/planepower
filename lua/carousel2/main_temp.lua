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
function get_carousel_setpoint()
	return siemensSensors:provides("data"):last()['carouselSpeedSetpoint']
end

require "math"

softlimit = 3.1415 -- Rad/s

-- ALWAYS check the return value of this!
function ramp_to(targetSpeed)
	acceleration = .1
	dt = .5 -- s
	threshold = .05 -- Rad/s
	if (math.abs(targetSpeed) > softlimit) then
		print "Requested speed is outside the soft limit!"
		return 0
	end
	while true do
		currentSpeed = get_carousel_speed()
		if math.abs(currentSpeed - targetSpeed) < threshold then
			set_carousel_speed(targetSpeed)
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

-- functionGenerator related functions

function set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase)
	
	set_property("functionGenerator","type",functionType)
	set_property("functionGenerator","amplitude",amplitude)
	set_property("functionGenerator","phase",phase)
	set_property("functionGenerator","offset",offset)
	set_property("functionGenerator","frequency",frequency)
	set_property("functionGenerator","whichDrive",whichDrive)
end

-- Stepping from zero to some value
function start_stepping()
	--Set the parameterf of our function generator for a step response
	--stepheight = 3.141/2000 -- Rad/s
	stepheight = 3.141/10 -- Rad/s
	lowtime = 1 -- seconds.  This is also the hightime.  Make longer than your settling time.
	functionType = 1 -- for square wave
	whichDrive = 1 -- for carousel
	amplitude = stepheight/2.0
	phase =3.2 -- a bit more than PI to make sure we start at 0
	offset = amplitude
	period = 2.0*lowtime
	frequency = 1.0/period

	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase)
	siemensActuators:start()
	functionGenerator:start()
end
function stop_stepping()
	functionGenerator:stop()
end

function step_around_current_setpoint()
	--Set the parameterf of our function generator for a step response
	stepheight = 3.141/20 -- Rad/s
	lowtime = 4.0 -- seconds.  This is also the hightime.  Make longer than your settling time.

	functionType = 1 -- for square wave
	whichDrive = 1 -- for carousel
	amplitude = stepheight/2.0
	phase =3.2 -- a bit more than PI to make sure we start at 0
	offset = get_carousel_setpoint()
	period = 2.0*lowtime
	frequency = 1.0/period

	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase)
	siemensActuators:start() -- make sure actuator are running
	functionGenerator:start() -- start!!
end

function sin_around_current_setpoint(amplitude,frequency)
	--Set the parameterf of our function generator for a step response
	functionType = 0 -- for sin wave
	whichDrive = 1 -- for carousel
	phase = 0 
	offset = get_carousel_setpoint()

	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase)
	siemensActuators:start()
	functionGenerator:stop() -- make sure the sin will start at currentspeed to avoid jumps 
	functionGenerator:start()
end

function sin_around_offset(offset,amplitude,frequency)
	ramp_to(offset) -- avoid jumps
	sin_around_current_setpoint(amplitude,frequency)
end

function stop_FunctionGenerator_and_ramp_to_0()
	--safe stop of the functionGenerator
	functionGenerator:stop()
	ramp_to(0)
	set_functionGenerator_properties(0,0,0,0,0,0)
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

function run_step_experiment()
	print "Running experimint NAOOOO!"
	sleep(.5)
	start_stepping()
	sleep(21)
	stop_stepping()
	os.exit()
	print "Exiting"
end

function run_offset_sin_experiment()
	print "Running experimint NAOOOO!"
	sleep(.5)
	sin_around_offset(2, -- offset
					.05, -- amplitude
					.5) -- frequency
	sleep(21)
	stop_FunctionGenerator_and_ramp_to_0()
	os.exit()
	print "Exiting"
end

--run_step_experiment()
run_offset_sin_experiment()

dofile("../shared/postamble.lua")
