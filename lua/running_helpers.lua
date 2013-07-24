#!/usr/bin/env rttlua-i

local P = {}
running_helpers = P

-- These lua functions make writing running scripts easier
function P.set_voltage(voltage)
	voltageController:setVoltage(0,voltage)
end

return running_helpers

