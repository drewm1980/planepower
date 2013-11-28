#!/usr/bin/env rttlua-i

-- Return a list of strings containing just the names of the output
-- ports of a component
function get_output_ports(componentName)
	c = _G[componentName]
	--print(componentName)
	pns = _G[componentName]:getPortNames()
	outputPortNames = {}
	for k,pn in ipairs(pns) do
		p = c:getPort(pn)
		pstring = tostring(p)
		if string.find(pstring,"out",1) then
			--print("Detected output port ".. pn)
			table.insert(outputPortNames,pn)
		end
	end
	return outputPortNames
end

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
	componentName = reportedComponentNames[i]
	deployer:connectPeers(reporterName,componentName)
	r = _G[reporterName]
	outputPortNames = get_output_ports(componentName)
	for k,portName in ipairs(outputPortNames) do
		r:reportPort(componentName, portName)
		--print(reporterName.." is reporting component "..componentName.." port "..portName)
	end
	deployer:setActivity(reporterName,0.0,reporterPrio,ORO_SCHED_RT)
	r:configure()
	r:start()
end

