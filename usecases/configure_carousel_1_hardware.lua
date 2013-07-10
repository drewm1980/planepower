#!/usr/bin/env rttlua-i
require "deployment_helpers"

PLANEPOWER="../"
PROPERTIES=PLANEPOWER.."properties/"


---------------------- Reporters

-- Set reporter policy
rp = rtt.Variable("ConnPolicy")
rp.type = 2
rp.size = 4096

sensorNames={"imu,"camera","encoder","lineAngle"}
reporterNames={"imuReporter","cameraReporter","encoderReporter",

imuReporter.ReportFile = "imuData.nc"
imuReporter.ReportPolicy = rp
imuReporter.ReportOnlyNewData = false
cameraReporter.ReportFile = "cameraData.nc"
cameraReporter.ReportPolicy = rp
cameraReporter.ReportOnlyNewData = false
encoderReporter.ReportFile = "encoderData.nc"
encoderReporter.ReportPolicy = rp
encoderReporter.ReportOnlyNewData = false
lineAngleReporter.ReportFile = "angleData.nc"
lineAngleReporter.ReportPolicy = rp
lineAngleReporter.ReportOnlyNewData = false

--]]
