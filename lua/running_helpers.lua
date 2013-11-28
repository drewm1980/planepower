#!/usr/bin/env rttlua-i

local P = {}
running_helpers = P

-- These lua functions make writing running scripts easier
function P.set_voltage(voltage)
	voltageController:setVoltage(0,voltage)
end

-- Return carousel arm rotation in rpm
function P.get_ddelta()
	return encoder:provides("data"):last()["omega_filt_rpm"]
end

return running_helpers

