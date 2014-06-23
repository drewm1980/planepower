#!/usr/bin/env rttlua-i

-- Return a list of strings containing just the names of the output
-- ports of a component
require "deployment_helpers"
for i,symbol in ipairs({"set_up_reporters"}) do
	_G[symbol] = deployment_helpers[symbol]
end

--Make sure the typekits are all loaded
--typekitNames = {"SiemensDriveStateTypekit.so",
				--"LineAnglesTypekit",
				--"ResampledMeasurementsTypekit",
				--"SiemensDriveCommandTypekit",
				--"ControllerGainsTypekit"}
--for i,name in ipairs(typekitNames) do
	----deployer:loadLibrary(name);
	--deployer:import(name);
--end

-- Load and set up a reporter for each sensor type
reportedComponentNames={"siemensSensors",
						"lineAngleSensor2",
						"resampler",
						--"gainLoader", -- sent more frequently from controller
						"controller"}

reporterBaseNames=reportedComponentNames -- shallow copy
set_up_reporters(reporterBaseNames,reportedComponentNames)
