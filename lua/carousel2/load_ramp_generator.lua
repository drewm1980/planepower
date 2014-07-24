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

load_component("rampGenerator","RampGenerator","rampGenerator")

----------------- Set Priorities and activities
deployer:setActivityOnCPU("rampGenerator", 0.1, controllerPrio, scheduler,quietCore)

cp = rtt.Variable("ConnPolicy")

deployer:connect("siemensSensors.data","rampGenerator.driveState",cp)
deployer:connect("rampGenerator.driveCommand","siemensActuators.controls",cp)


-- deployer:connect("siemensSensors.triggerOut","rampGenerator.triggerIn",cp)

--------------- Configure and start the components
rampGenerator:configure()
