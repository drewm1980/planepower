-- WARNING!! These values also get parsed out of this file by 
-- plot_steady_states.py, so if you change the names of the variables,
-- fix them there as well!
takeoffSpeed = 1.10-- Rad/s, a bit before takeoff.
takeoffAngle = -1.2566-- Radians
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

function set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase,numberOfSines)
	
	set_property("functionGenerator","type",functionType)
	set_property("functionGenerator","amplitude",amplitude)
	set_property("functionGenerator","phase",phase)
	set_property("functionGenerator","offset",offset)
	set_property("functionGenerator","frequency",frequency)
	set_property("functionGenerator","whichDrive",whichDrive)
	set_property("functionGenerator","numberOfSines", numberOfSines)
end


function set_functionGenerator_for_multisine(functionType,amplitude,offset,frequency,numberOfSines,wStar,wEnd)
	
	set_property("functionGenerator","type",functionType)
	set_property("functionGenerator","amplitude",amplitude)
	set_property("functionGenerator","phase",0)
	set_property("functionGenerator","offset",offset)
	set_property("functionGenerator","frequency",frequency)
	set_property("functionGenerator","whichDrive",1)
	set_property("functionGenerator","numberOfSines", numberOfSines)
	set_property("functionGenerator","wStart", wStart)
	set_property("functionGenerator","wEnd", wEnd)
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

	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase,0)
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

	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase,0)
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

	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase,0)
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
	set_functionGenerator_properties(0,0,0,0,0,0,0)
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

	changingGainsOnline = false

	-- A conservative choice of heights, working pretty well.
	h_high = lookup_steady_state_elevation(normalFlyingSpeed+normalStepHeight/2.0)
	h_low = lookup_steady_state_elevation(normalFlyingSpeed-normalStepHeight/2.0)

	-- A more aggressive choice of hights, not working so well.
	--h_high = -40.0*math.pi/180.0
	--h_low = -55.0*math.pi/180.0

	speed_high = lookup_steady_state_speed(h_high)
	--if math.isnan(speed_high) then
		--print "experiment_helpers: lookup failed on h_high"
		--return
	--end
	speed_low = lookup_steady_state_speed(h_low)
	--if math.isnan(speed_low) then
		--print "experiment_helpers: lookup failed on h_low"
		--return
	--end
	print(tostring(speed_high))
	print(tostring(speed_low))
	fast_ramp(speed_low)

	stepHeight = h_high-h_low -- Radians

	-- Set up Function Generator
	functionType = 1 -- for square wave
	whichDrive = 1 -- for carousel, but also currently needed for controller reference
	amplitude = stepHeight/2.0
	phase = 0.0001 
	offset = h_low + stepHeight/2.0 -- Radians
	lowtime = 20 
	period = 2.0*lowtime
	frequency = 1.0/period
	periods = 4 -- number of periods to run for
	set_functionGenerator_properties(functionType,whichDrive,amplitude,offset,frequency,phase,0)

	-- Start the Experiment
	siemensActuators:start()
	functionGenerator:stop()
	functionGenerator:start() -- has to be before controller is started!
	sleep(1) -- TODO: Figure out why it takes a long time to get measurement data!

	set_pid_gains(0,0,0)

	controller:start() -- This is where the integrator is reset
	sleep(.1) -- Time for the controller state to settle

	set_property("controller","freezeFeedForwardTerm",true) -- Needs to be after you start controller so the controller has a reasonable feedforward term to freeze.
	--set_property("controller","freezeFeedForwardTerm",false)

	if not changingGainsOnline then
		sleep(0.5*period) -- Get past any transient from the ramp
		set_pid_gains(.42,.65,.44)
		sleep(periods*period)
		fast_ramp(0)
	end
end

function run_log_multisine_experiment()

	print "Running log  multisine experiment..."
	if runningOpenLoop then
		
		-- set up function generator
		functionType = 3 -- for log multisine
		amplitude = 0.005 -- rad/s !!always test without activated drives first!!
		numberOfDecades = 2 -- 10^0 to 100 
		offset = normalFlyingSpeed -- radians
		frequency = 0.1/(2*PI) -- hz. the lowest frequency in the multisine
		numberOfSines = 20
		wStart = 0.2
		wEnd = 20
		periods = 12 -- number of periods to run for
		
		-- start the experiment
		siemensActuators:start()
		--always ramp before setting the functiongenerator properties
		fast_ramp(offset)
		set_functionGenerator_for_multisine(functionType,amplitude,offset,frequency,numberOfSines,wStart,wEnd)

		functionGenerator:start()
		sleep(periods/frequency)
		fast_ramp(0)
	else
		print "Running open loop not activated! Visit main.lua to change it."
	end
end

function run_multisine_experiment()

	print "Running multisine experiment..."
	if runningOpenLoop then
		
		-- set up function generator
		functionType = 2 -- for linear multisine	
		amplitude = 0.01 -- rad/s !!multiplies with numberofsines/2!!
		offset = normalFlyingSpeed -- radians
		frequency = 0.1/(2*PI) -- hz. the lowest frequency in the multisine
		numberOfSines = 20 
		wStart = 1
		wEnd = 3
		periods = 12 -- number of periods to run for
		
		-- start the experiment
		siemensActuators:start()
		--always ramp before setting the functiongenerator properties
		fast_ramp(offset)
		set_functionGenerator_for_multisine(functionType,amplitude,offset,frequency,numberOfSines,wStart,wEnd)

		functionGenerator:start()
		sleep(periods/frequency)
		fast_ramp(0)
	else
		print "Running open loop not activated! Visit main.lua to change it."
	end
end
