#!/usr/bin/env rttlua-i

rtt.logl('Info', "Configuring Carousel 1 Hardware...")

-- MCU
deployer:loadService("mcuHandler","marshalling")
mcuHandler.marshalling.loadProperties("../../properties/tcp.cpf")
--mcuHandler.Ts = 1.0 / base_hz
--mcuHandler.rtMode = true

-- Encoder
--encoder.encoderPort = 0;
-- Same as for SOEM
--encoder.Ts = 0.001;

-- LED tracker - hangs until frame arrival, does processing, and re-triggers itself.
--LEDTracker.useExternalTrigger = true;
--LEDTracker.sigma_marker = 20;

--
-- Reporters
--

-- Set reporter policy
rp = rtt.Variable("ConnPolicy")
rp.type = 2
rp.size = 4096

--imuReporter.ReportFile = "imuData.nc"
--imuReporter.ReportPolicy = rp
--imuReporter.ReportOnlyNewData = false
--cameraReporter.ReportFile = "cameraData.nc"
--cameraReporter.ReportPolicy = rp
--cameraReporter.ReportOnlyNewData = false
--encoderReporter.ReportFile = "encoderData.nc"
--encoderReporter.ReportPolicy = rp
--encoderReporter.ReportOnlyNewData = false
--lineAngleReporter.ReportFile = "angleData.nc"
--lineAngleReporter.ReportPolicy = rp
--lineAngleReporter.ReportOnlyNewData = false
