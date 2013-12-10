#!/usr/bin/env rttlua-i
dofile("preamble.lua")

require "deployment_helpers"
for i,symbol in ipairs({"load_component",
						"load_properties",
						"get_property",
						"set_property"}) do
	_G[symbol] = deployment_helpers[symbol]
end

dofile("load_carousel_1_hardware.lua")
dofile("setup_carousel_1_reporters.lua")

load_component("DynamicMhe","DynamicMhe","mhe")
load_properties("mhe",PROPERTIES.."DynamicMhe.cpf")

ocpPrio = 80


masterTimer:start()

--os.execute("sleep " .. 1)


dofile("postamble.lua")
