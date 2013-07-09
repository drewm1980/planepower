#!/usr/bin/env rttlua-i
require "deployment_helpers"

dofile("preamble.lua")
dofile("load_soem.lua") -- note, still figuring out how to set Properties...

libraryNames={"mcuHandler","encoder","cameraTrigger","lineAngleSensor","LEDTracker"}
classNames={  "McuHandler","Encoder","CameraTrigger","LineAngleSensor","LEDTracker"}
instanceNames=libraryNames

for i=1,#libraryNames do
	deployment_helpers.load_component(libraryNames[i],classNames[i],instanceNames[i])
end

-- Load Reporters with Milan's names
reporterBaseNames={"imu","camera","encoder","lineAngle"}
reporterNames={}
for i=1,#reporterBaseNames do 
	reporterNames[i]=reporterBaseNames[i].."Reporter"
end
for key,reporterName in pairs(reporterNames) do
	deployment_helpers.load_reporter(reporterName)
end

