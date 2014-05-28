#!/usr/bin/env rttlua-i

require("rttlib")
rtt.setLogLevel("Warning")

require("io") -- enable loops to be broken with keyboard
enterPressed = false -- global variable to enable detection of loop broken by return

rttlib.color = true
tc = rtt.getTC() -- A handle to the TaskContext (C++ component) that is running our interpreter (?)
deployer = tc:getPeer("Deployer") -- A handle to the component that is responsible for loading new components,etc

-- Load libraries and instantiate components
rtt.logl("Info", "Loading PLANEPOWER components...")

deployer:import("ocl")

ORO_SCHED_RT=rtt.globals.ORO_SCHED_RT
ORO_SCHED_OTHER=rtt.globals.ORO_SCHED_OTHER



