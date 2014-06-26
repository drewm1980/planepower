#!/usr/bin/env rttlua-i

----- Note: for now, you must manually keep this order consistent with the same variable in carousel2.py
telemetryInstanceNames={"siemensSensorsTelemetry",	  "lineAngleSensor2Telemetry", "resampledMeasurementsTelemetry", "controllerTelemetry"}
telemetryClassNames={   "SiemensDriveStateTelemetry", "LineAnglesTelemetry"      , "ResampledMeasurementsTelemetry", "SiemensDriveCommandTelemetry"}
telemetryInstances={}

-- Our telemetry components are not in a path that orocos' ComponentLoader base class (which deployer is presumably a child of) can find, apparently, so we need to load the libraries explicitly.
-- I have no clue why this wasn't necessary for the typekits.
deployer:import("../../components/carousel2/siemensSensors/types/SiemensDriveState.so")
deployer:import("../../components/carousel2/gainLoader/types/ControllerGains.so")
deployer:import("../../components/carousel2/lineAngleSensor2/types/LineAngles.so")
deployer:import("../../components/carousel2/resampler/types/ResampledMeasurements.so")
deployer:import("../../components/carousel2/siemensActuators/types/SiemensDriveCommand.so")

for i=1,#telemetryInstanceNames do
	--deployer:import(telemetryClassNames[i])
	deployer:loadComponent(telemetryInstanceNames[i],telemetryClassNames[i])
	_G[telemetryInstanceNames[i]]=deployer:getPeer(telemetryInstanceNames[i])
	telemetryInstances[i] = _G[telemetryInstanceNames[i]]
end

startPort = 5562
for i=1,#telemetryInstances do
	portNumber = startPort+i-1 -- because lua is one indexed
	portString = "tcp://*:"..portNumber
	print ("Starting "..telemetryInstanceNames[i].." on port "..portString.."..")
	telemetryInstances[i]:getProperty("port"):set(portString)
end
--mcuHandlerTelemetry:getProperty("port"):set("tcp://*:5563")

-- Synchronize the start time for all of the telemetry components
deployer:import( "HighwindStuff" )
rtt.provides():require('HighwindServices')
startTime = rtt.provides('HighwindServices'):getTimestamp()
for i=1,#telemetryInstanceNames do
	telemetryInstances[i]:getProperty("startTime"):set(startTime)
end
--mcuHandlerTelemetry:getProperty("startTime"):set(startTime)

for i=1,#telemetryInstanceNames do
	deployer:setActivityOnCPU(telemetryInstanceNames[i], 1.0/telemetryFrequency, telemetryPrio, scheduler, someNoisyCore)
end
--deployer:setActivityOnCPU("mcuHandlerTelemetry", 1.0 / 100.0, telemetryPrio, ORO_SCHED_OTHER, 1)

cp = rtt.Variable("ConnPolicy")
deployer:connect("siemensSensors.data","siemensSensorsTelemetry.msgData", cp)
deployer:connect("lineAngleSensor2.data","lineAngleSensor2Telemetry.msgData", cp)
if measuringStepResponses then
	deployer:connect("functionGenerator.data","controllerTelemetry.msgData", cp)
else
	deployer:connect("resampler.data","resampledMeasurementsTelemetry.msgData", cp)
	deployer:connect("controller.data","controllerTelemetry.msgData", cp)
end

for i=1,#telemetryInstanceNames do
	telemetryInstances[i]:configure()
	telemetryInstances[i]:start()
end

