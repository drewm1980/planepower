#!/usr/bin/env rttlua-i
dofile("main.lua") 

which_experiment = 0

if which_experiment == 0 then
	print "No experiment selected!"
	print "You are in 'free lua mode'. If you don't exactly know what you are doing close it! (Ctrl + D)"
elseif which_experiment == 1 then
	run()
elseif which_experiment == 2 then
	run_step_experiment()
elseif which_experiment == 3 then
	run_offset_sin_experiment()
elseif which_experiment == 4 then
	run_offset_step_experiment()
elseif which_experiment == 5 then
	run_slow_ramp_to_max_and_back_to_0_experiment()
elseif which_experiment == 6 then
	run_very_slow_ramp_to_max_and_back_to_0_experiment()
elseif which_experiment == 7 then
	run_rampGenerator_test()
else 
	print "No experiment selected!"
	print "Exiting"
	os.exit()

