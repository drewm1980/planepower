#!/usr/bin/env rttlua-i

-- Return a list of strings containing just the names of the output
-- ports of a component
require "deployment_helpers"
for i,symbol in ipairs({"set_up_reporters"}) do
	_G[symbol] = deployment_helpers[symbol]
end

-- NOTE!!!  MAKE SURE THE DIRECTORIES THAT CONTAIN THE "types" DIRECTORIES
-- ARE ON YOUR RTT_COMPONENT_PATH environment variable, otherwise only one
-- typekit will be loaded at random (?), and you will be confused as fuck.

-- The typekits will still work in the components for some reason, but
-- the reporters will not start!

-- Load and set up a reporter for each sensor type
reportedComponentNames={"siemensSensors",
						"lineAngleSensor2",
						"resampler",
						--"gainLoader", -- sent more frequently from controller
						"controller"}

reporterBaseNames=reportedComponentNames -- shallow copy
set_up_reporters(reporterBaseNames,reportedComponentNames)
