#!/usr/bin/env rttlua-i

local P = {}
deployment_helpers = P

-- These lua functions make writing deployment scripts easier
function P.load_component(libraryName,className,instanceName)
	print("Loading "..libraryName.." Component...")
	deployer:import(libraryName)
	deployer:loadComponent(instanceName,className)
	deployer:loadService(instanceName,"marshalling")
	_G[instanceName]=deployer:getPeer(instanceName)
end

function P.load_reporter(reporterName)
	deployer:loadComponent(reporterName, "OCL::NetcdfReporting")
	reporter=deployer:getPeer(reporterName)
	_G[reporterName]=reporter
	return 
end

function P.load_properties(instanceName,propertiesFilename)
	return _G[instanceName]:provides("marshalling"):getOperation("loadProperties")(propertiesFilename)
end

function P.set_property(instanceName, propertyName, propertyValue)
	return _G[instanceName]:getProperty(propertyName):set(propertyValue)
end
function P.get_property(instanceName, propertyName, propertyValue)
	return _G[instanceName]:getProperty(propertyName):get(propertyValue)
end

function P.set_up_reporters(reporterBaseNames,reportedComponentNames)
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
end

return deployment_helpers

