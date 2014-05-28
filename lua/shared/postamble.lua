#!/usr/bin/env rttlua-i

---- Niceties for when we run in interactive mode
require "complete"
require "readline"
function i()
	rttlib.info()
	rttlib.stat()
end
i()
