#!/usr/bin/env rttlua-i

dofile("preamble.lua")

deployer:import("masterTimer")
deployer:loadComponent("masterTimer","MasterTimer")
masterTimer=deployer:getPeer("masterTimer")

dofile("load_carousel_1_hardware.lua")
dofile("configure_carousel_1_hardware.lua")

-- Master timer base clock is clock of our fastest running sensor
deployer:loadService("masterTimer", "marshalling")
masterTimer.marshalling.loadProperties("../../properties/masterTimer.cpf")
--base_hz = masterTimer.imu_target_hz

----------------- Set Priorities and activities
masterTimerPrio = 98
sensorPrio = 97
ocpPrio = 80
LEDTrackerPrio = 70

deployer:setActivity("masterTimer", 1.0 / base_hz, masterTimerPrio, ORO_SCHED_RT)
deployer:setActivity("mcuHandler", 0.002, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("voltageController", 0.01, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("encoder", 0.0, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("cameraTrigger", 0.0, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("lineAngleSensor", 0.0, sensorPrio, ORO_SCHED_RT)
deployer:setActivity("LEDTracker", 0.0, LEDTrackerPrio, ORO_SCHED_RT)
--deployer:setActivity("poseFromMarkers", 0.0, LEDTrackerPrio, ORO_SCHED_RT)

deployer:setActivity("imuReporter", 0.0, LowestPriority, ORO_SCHED_RT)
deployer:setActivity("cameraReporter", 0.0, LowestPriority, ORO_SCHED_RT)
deployer:setActivity("encoderReporter", 0.0, LowestPriority, ORO_SCHED_RT)
deployer:setActivity("lineAngleReporter", 0.0, LowestPriority, ORO_SCHED_RT)

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

dofile("postamble.lua")
