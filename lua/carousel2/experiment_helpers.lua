-- WARNING!! These values also get parsed out of this file by 
-- plot_steady_states.py, so if you change the names of the variables,
-- fix them there as well!
takeoffSpeed = 1.0 -- Rad/s, a bit before takeoff.
takeoffAngle = -1.1344 -- Radians
turbulentSpeed = 2.4 -- Rad/s . speed above which the ball starts moving eratically
normalFlyingSpeed = 1.6 
normalStepHeight = .12 -- Rad/s

softlimit = PI -- Rad/s
elevationJumpingSpeed = 1.84 -- Rad/s, speed at which (+-0.1) the elevation angle jumps ca 4 deg

require "math"

function set_carousel_speed(speed)
	if speed==nil then
		print "Speed cannot be nil!!!"
		return
	end
	siemensActuators:stop()
	siemensActuators:getOperation('setCarouselSpeed')(speed)
end

function get_carousel_speed()
	return siemensSensors:provides("data"):last()['carouselSpeedSmoothed']
end

function get_carousel_setpoint()
	return siemensSensors:provides("data"):last()['carouselSpeedSetpoint']
end

function get_rampGenerator_rampstatus()
	return rampGenerator:provides("info"):last()
end


-- rampGenerator related functions
function wait_till_ramp_is_done()
	str = get_rampGenerator_rampstatus()
	oldstr = ""
	running = true
	while running do
		sleep(0.1)
		str = get_rampGenerator_rampstatus()
		-- stop spamming 
		if oldstr == str then
		else
			print( str )
			oldstr = str
		end
		if  str == "Ramp goal achieved! Stoping rampGenerator..." then
			running = false
		end 
	end
	rampGenerator:stop()
	set_property("rampGenerator","acceleration",0.1)
	set_property("rampGenerator","targetSpeed",0)
	print("done.")
end

function ramp_with(targetSpeed,acceleration)
	if runningOpenLoop then
		print("Stoping function generator...")
		stop_functionGenerator()
	else
		print("Stoping controller...")
		controller:stop()
	end	
	print("Ramping to "..tostring(targetSpeed))
	set_property("rampGenerator","acceleration",acceleration)
	set_property("rampGenerator","targetSpeed",targetSpeed)
	rampGenerator:start()
	wait_till_ramp_is_done()
end

function slow_ramp(targetSpeed)
	acceleration = 0.01
	ramp_with(targetSpeed,acceleration)
end

function fast_ramp(targetSpeed)
	acceleration = 0.1
	ramp_with(targetSpeed,acceleration)
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
	--stepHeight = 3.141/2000 -- Rad/s
	stepHeight = PI/10 -- Rad/s
	lowtime = 1 -- seconds.  This is also the hightime.  Make longer than your settling time.
	functionType = 1 -- for square wave
	whichDrive = 1 -- for carousel
	amplitude = stepHeight/2.0
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

function step_around_current_setpoint(stepHeight,lowtime)
	--Set the parameterf of our function generator for a step response
	--stepHeight = 3.141/20 -- Rad/s
	--lowtime = 4.0 -- seconds.  This is also the hightime.  Make longer than your settling time.

	functionType = 1 -- for square wave
	whichDrive = 1 -- for carousel
	amplitude = stepHeight/2.0
	phase = 3.1416 -- a bit more than PI to make sure we start at 0
	offset = get_carousel_setpoint()
	period = 2.0*lowtime
	frequency = 1.0/period

	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase)
	siemensActuators:start() -- make sure actuator are running
	functionGenerator:stop() -- make sure the sin will start at currentspeed to avoid jumps 
	print "Starting function generator"
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
	print "Starting function generator"
	functionGenerator:start()
end

function sin_around_offset(offset,amplitude,frequency)
	fast_ramp(offset) -- avoid jumps
	sin_around_current_setpoint(amplitude,frequency)
end

function step_around_offset(offset,stepHeight,lowtime)
	fast_ramp(offset) -- avoid jumps
	step_around_current_setpoint(stepHeight,lowtime)
end

function stop_functionGenerator()
	--safe stop of the functionGenerator
	functionGenerator:stop()
	set_functionGenerator_properties(0,0,0,0,0,0)
end
----------------- THE EXPERIMENTS!!!!!!! -------------
function run()
	speedOffset = softlimit/2.0
	if (fast_ramp(speedOffset)) then
		fast_ramp(0.0)
		return 0
	end
	sleep(10)
	fast_ramp(0.0)
end

function run_step_experiment()
	print "Running experimint NAOOOO!"
	sleep(.5)
	start_stepping()
	sleep(21)
	stop_stepping()
end

function run_offset_sin_experiment()
	fast_ramp(normalFlyingSpeed) -- Just cause we don't want this included in the sleep time
	frequency = 0.3 -- Hz
	sin_around_offset(normalFlyingSpeed, -- offset
					.07, -- amplitude
					frequency) -- frequency
	periods = 8
	sleeptime = 8.0*1.0/frequency
	print ("Going to sleep for "..tostring(sleeptime).." seconds while function generator runs...")
	sleep(sleeptime)
	fast_ramp(0)
end

function run_offset_step_experiment()
	lowtime = 16
	stepHeight = normalStepHeight
	step_around_offset(normalFlyingSpeed, -- offset
					stepHeight, -- stepHeight
					lowtime) -- lowtime
	periods = 4
	sleep(periods*lowtime*2)
	fast_ramp(0)
end

function run_steady_state_experiment()
	fast_ramp(takeoffSpeed)
	acceleration = .005 -- in rad/s^2
	ramp_with(	turbulentSpeed, -- targetSpeed
			acceleration) -- acceleration
	sleep(10)
	ramp_with(	takeoffSpeed, -- targetSpeed
			acceleration) -- acceleration
	fast_ramp(0)
end

function run_rampGenerator_test()
	
	print "Running experimint NAOOOO!"
	sleep(.5)
	set_property("rampGenerator","acceleration",0.1)
	set_property("rampGenerator","targetSpeed",0.5)
	rampGenerator:start()
	for i=1,100 do
		rampGenerator:stat()
		sleep(.5)
	end
	set_property("rampGenerator","targetSpeed",0.0)
	for i=1,100 do
		rampGenerator:stat()
		sleep(.5)
	end
end

function run_ramp_around_jump_experiment()
	print "Running experimint NAOOOO!"
	dspeed = 0.2
	fast_ramp(elevationJumpingSpeed + dspeed)
	slow_ramp(elevationJumpingSpeed - dspeed)
	slow_ramp(elevationJumpingSpeed + dspeed)
	slow_ramp(elevationJumpingSpeed - dspeed)
	slow_ramp(elevationJumpingSpeed + dspeed)
	fast_ramp(0)
end	

function run_pid_experiment()
	print "Running PID experiment..."

	sanityCheck=true -- For this the MACHINE SHOULD BE TURNED OFF!!!
	if not sanityCheck then
		fast_ramp(normalFlyingSpeed)
	end
	h1 = lookup_steady_state_elevation(normalFlyingSpeed+normalStepHeight/2.0)
	h2 = lookup_steady_state_elevation(normalFlyingSpeed-normalStepHeight/2.0)
	stepHeight = h1-h2 -- Radians

	-- Set up Function Generator
	functionType = 1 -- for square wave
	whichDrive = 1 -- for carousel, but also currently needed for controller reference
	amplitude = stepHeight/2.0
	phase = 3.1416 -- a bit more than PI to make sure we start at 0
	offset = lookup_steady_state_elevation(normalFlyingSpeed) -- Radians
	lowtime = 16
	period = 2.0*lowtime
	frequency = 1.0/period
	periods = 3 -- number of periods to run for
	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase)

	-- Start the Experiment
	siemensActuators:start()
	functionGenerator:stop()
	functionGenerator:start() -- has to be before controller is started!
	sleep(1) -- TODO: Figure out why it takes a long time to get measurement data!

	set_pid_gains(.00001,0,0)
	controller:start()
	sleep(.1)
	set_property("controller","freezeFeedForwardTerm",true)

	if not sanityCheck then
		sleep(periods*lowtime*2)
		fast_ramp(0)
	end
end


