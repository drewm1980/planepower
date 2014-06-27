#!/usr/bin/env rttlua-i

---- Niceties for when we run in interactive mode
require "complete"
require "readline"
function stat()
	rttlib.info()
	rttlib.stat()
end
stat()
