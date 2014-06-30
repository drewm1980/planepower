#!/usr/bin/env rttlua-i
require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_reporter",
						"load_properties",
						"store_properties",
						"get_property",
						"set_property",
						"sleep"}) do
	_G[symbol] = deployment_helpers[symbol]
end

PLANEPOWER="../../"

load_component("functionGenerator","FunctionGenerator","functionGenerator")

freeRunningFunctionGenerator = false

----------------- Set Priorities and activities
if freeRunningFunctionGenerator then
	updateFrequency = 40
	deployer:setActivityOnCPU("functionGenerator", 1.0/updateFrequency, controllerPrio, scheduler,quietCore)
else
	deployer:setActivityOnCPU("functionGenerator", 0.0, controllerPrio, scheduler,quietCore)
end

cp = rtt.Variable("ConnPolicy")
deployer:connect("functionGenerator.data","siemensActuators.controls",cp)

if not freeRunningFunctionGenerator then
	deployer:connect("siemensSensors.triggerOut","functionGenerator.triggerIn",cp)
end

--------------- Configure and start the components
functionGenerator:configure()

