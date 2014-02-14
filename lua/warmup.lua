#!/usr/bin/env rttlua
-- Sometimes the plane servos make an annoying noise when they get
-- stuck just slightly uncentered.  This is harmless, but you can
-- run this script to wiggle the servos just a bit to stop the noise.

dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property",
						"sleep"}) do
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
--dofile("setup_carousel_1_reporters.lua")
dofile("setup_carousel_1_telemetry.lua")

masterTimer:start()

warmup()
slowdown()


