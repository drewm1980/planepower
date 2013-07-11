#!/usr/bin/env rttlua-i
require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property"}) do
	_G[symbol] = deployment_helpers[symbol]
end

dofile("preamble.lua")

PLANEPOWER="../"
PROPERTIES=PLANEPOWER.."properties/"

soemPrio=99
masterTimerPrio = 98
sensorPrio = 97
ocpPrio = 80
LEDTrackerPrio = 70
reporterPrio = 50

load_component("masterTimer","MasterTimer","masterTimer")
load_properties("masterTimer",PROPERTIES.."masterTimer.cpf")
base_hz = get_property("masterTimer","imu_target_hz")

dofile("load_carousel_1_hardware.lua")

--[[

----------------- Set Priorities and activities

deployer:setActivity("masterTimer", 1.0 / base_hz, masterTimerPrio, ORO_SCHED_RT)
deployer:setActivity("mcuHandler", 0.002, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("voltageController", 0.01, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("encoder", 0.0, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("cameraTrigger", 0.0, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("lineAngleSensor", 0.0, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("LEDTracker", 0.0, LEDTrackerPrio, ORO_SCHED_RT)
--deployer:setActivity("poseFromMarkers", 0.0, LEDTrackerPrio, ORO_SCHED_RT)

---------------- Connect Components
cp = rtt.Variable("ConnPolicy")
cpLT = rtt.Variable("ConnPolicy")
cpLT.type = 1
cpLT.size = 5

-- deployer:connect("masterTimer.imuClock","mcuHandler.trigger", cp)
deployer:connect("masterTimer.cameraClock", "LEDTracker.triggerTimeStampIn", cpLT)
deployer:connect("masterTimer.cameraClock", "cameraTrigger.Trigger", cp)
--deployer:connect("LEDTracker.markerPositions", "poseFromMarkers.markerPositions", cp)

connect("voltageController.eboxAnalog", "soemMaster.Slave_1001.AnalogIn", cp)
connect("soemMaster.Slave_1001.Measurements", "encoder.eboxOut", cp)
connect("soemMaster.Slave_1001.Measurements", "lineAngleSensor.eboxOut", cp)

---------------- Connect Reporters

deployer:connectPeers("imuReporter", "mcuHandler")
imuReporter.reportComponent( "mcuHandler" )

deployer:connectPeers("cameraReporter", "LEDTracker")
cameraReporter.reportComponent( "LEDTracker" )

deployer:connectPeers("encoderReporter", "encoder")
encoderReporter.reportComponent( "encoder" )

deployer:connectPeers("lineAngleReporter", "lineAngleSensor")
lineAngleReporter.reportComponent( "lineAngleSensor" )

--------------- Configure and start the components

masterTimer.configure()

mcuHandler.configure()
mcuHandler.start()

voltageController.configure()
voltageController.start()

encoder.configure()
encoder.start()

cameraTrigger.configure()
cameraTrigger.start()

lineAngleSensor.configure()
lineAngleSensor.start()

LEDTracker.configure()
LEDTracker.start()

imuReporter.configure()
imuReporter.start()

cameraReporter.configure()
cameraReporter.start()

encoderReporter.configure()
encoderReporter.start()

lineAngleReporter.configure()
lineAngleReporter.start()

-- Now, when all other components are started, start the master timer component.
masterTimer.start()

--]]

dofile("postamble.lua")
