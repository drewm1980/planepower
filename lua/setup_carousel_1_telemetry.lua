#!/usr/bin/env rttlua-i

telemetryComponentNames={"McuHandlerTelemetry","EncoderTelemetry","LineAngleSensorTelemetry","LEDTrackerTelemetry" }
telemetryBaseNames={"mcuHandlerTelemetry","encoderTelemetry","lineAngleSensorTelemetry","ledTrackerTelemetry" }

for i=1,#telemetryComponentNames do
	deployer:loadComponent(telemetryBaseNames[i],telemetryComponentNames[i])
	_G[telemetryBaseNames[i]]=deployer:getPeer(telemetryBaseNames[i])
end

mcuHandlerTelemetry:getProperty("port"):set("tcp://*:5563")
encoderTelemetry:getProperty("port"):set("tcp://*:5564")
ledTrackerTelemetry:getProperty("port"):set("tcp://*:5565")
lineAngleSensorTelemetry:getProperty("port"):set("tcp://*:5566")

deployer:import( "HighwindStuff" )
rtt.provides():require('HighwindServices')
startTime = rtt.provides('HighwindServices'):getTimestamp()

mcuHandlerTelemetry:getProperty("startTime"):set(startTime)
encoderTelemetry:getProperty("startTime"):set(startTime)
ledTrackerTelemetry:getProperty("startTime"):set(startTime)
lineAngleSensorTelemetry:getProperty("startTime"):set(startTime)

telemetryPrio = 0

deployer:setActivityOnCPU("mcuHandlerTelemetry", 1.0 / 100.0, telemetryPrio, ORO_SCHED_OTHER, 1)
deployer:setActivityOnCPU("encoderTelemetry", 1.0 / 100.0, telemetryPrio, ORO_SCHED_OTHER, 1)
deployer:setActivityOnCPU("ledTrackerTelemetry", 1.0 / 25.0, telemetryPrio, ORO_SCHED_OTHER, 1)
deployer:setActivityOnCPU("lineAngleSensorTelemetry", 1.0 / 100.0, telemetryPrio, ORO_SCHED_OTHER, 1)

cp = rtt.Variable("ConnPolicy")
deployer:connect("mcuHandler.data", "mcuHandlerTelemetry.msgData", cp)
deployer:connect("encoder.data", "encoderTelemetry.msgData", cp)
deployer:connect("LEDTracker.data", "ledTrackerTelemetry.msgData", cp)
deployer:connect("lineAngleSensor.data", "lineAngleSensorTelemetry.msgData", cp)

for i=1,#telemetryBaseNames do     
	_G[telemetryBaseNames[i]]:configure()
	_G[telemetryBaseNames[i]]:start()
end

