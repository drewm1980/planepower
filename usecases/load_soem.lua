#!/usr/bin/env rttlua-i

rtt.logl('Info', "Loading soem component...")

dofile("preamble.lua")

deployer:import("soem_master")
deployer:import("soem_ebox")
deployer:loadComponent("soemMaster","soem_master::SoemMasterComponent")
soemMaster=deployer:getPeer("soemMaster")
soemMaster:getProperty("ifname"):set("eth1")
soemPrio=99
deployer:setActivity("soemMaster", 0.001, soemPrio, ORO_SCHED_RT)

dofile("postamble.lua")


