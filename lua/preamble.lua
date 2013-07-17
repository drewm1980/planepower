#!/usr/bin/env rttlua-i

require("rttlib")
rtt.setLogLevel("Warning")

rttlib.color = true
tc = rtt.getTC()
deployer = tc:getPeer("Deployer")

-- Load libraries and instantiate components
rtt.logl("Info", "Loading PLANEPOWER components...")

deployer:import("ocl")

ORO_SCHED_RT=rtt.globals.ORO_SCHED_RT



