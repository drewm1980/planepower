#!/usr/bin/env rttlua-i

-- Load and set up a reporter for each sensor type
reporterBaseNames={"imu","camera","encoder","lineAngle"}
reportedComponentNames={"mcuHandler","LEDTracker","encoder","lineAngleSensor"}
reporterNames={}
reporterFileNames={}
for i=1,#reporterBaseNames do 
	reporterNames[i]=reporterBaseNames[i].."Reporter"
	reporterFileNames[i]=reporterBaseNames[i].."Data.nc"
end
rp = rtt.Variable("ConnPolicy")
rp.type = 2
rp.size = 4096
for i,reporterName in pairs(reporterNames) do
	load_reporter(reporterName)
	set_property(reporterName,"ReportFile",reporterFileNames[i])
	set_property(reporterName,"ReportPolicy",rp)
	set_property(reporterName,"ReportOnlyNewData",false)
	deployer:connectPeers(reporterName,reportedComponentNames[i])
	_G[reporterName]:reportComponent(reportedComponentNames[i])
	deployer:setActivity(reporterName,0.0,reporterPrio,ORO_SCHED_RT)
	_G[reporterName]:configure()
	_G[reporterName]:start()
end

