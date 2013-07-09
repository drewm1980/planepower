#!/usr/bin/env rttlua-i

local P = {}
deployment_helpers = P

-- These lua functions make writing deployment scripts easier
function P.load_component(libraryName,className,instanceName)
	print("Loading "..libraryName.." Component...")
	deployer:import(libraryName)
	deployer:loadComponent(instanceName,className)
	deployer:loadService(instanceName,"marshalling")
	_G[instanceName]=deployer:getPeer(libraryName)
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

return deployment_helpers

