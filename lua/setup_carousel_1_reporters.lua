#!/usr/bin/env rttlua-i

-- Return a list of strings containing just the names of the output
-- ports of a component
require "deployment_helpers"
for i,symbol in ipairs({"set_up_reporter"}) do
	_G[symbol] = deployment_helpers[symbol]
end

function get_output_ports(componentName)
	c = _G[componentName]
	--print(componentName)
	pns = _G[componentName]:getPortNames()
	outputPortNames = {}
	for k,pn in ipairs(pns) do
		p = c:getPort(pn)
		pstring = tostring(p)
		if string.find(pstring,"out",1) then
			--print("Detected output port ".. pn)
			table.insert(outputPortNames,pn)
		end
	end
	return outputPortNames
end

-- Load and set up a reporter for each sensor type
reporterBaseNames={"imu","camera","encoder","lineAngle"}
reportedComponentNames={"mcuHandler","LEDTracker","encoder","lineAngleSensor"}
set_up_reporters(reporterBaseNames,reportedComponentNames)
