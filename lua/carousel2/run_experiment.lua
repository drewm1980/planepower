#!/usr/bin/env rttlua-i
dofile("main.lua") 

which_experiment = 4

if which_experiment == 1 then
	run()
elseif which_experiment == 2 then
	run_step_experiment()
elseif which_experiment == 3 then
	run_offset_sin_experiment()
elseif which_experiment == 4 then
	run_slow_ramp_to_max_and_back_to_0_experiment()
else 
	print "No experiment selected!"
	os.exit()
	print "Exiting"
end
