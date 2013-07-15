#!/usr/bin/env rttlua-i

-- Part of the state machine example

dofile("../preamble.lua")

deployer:loadComponent("supervisor", "OCL::LuaComponent")
supervisor = deployer:getPeer("supervisor")
supervisor:exec_file("launch_fsm.lua")
supervisor:configure()

cmd = rttlib.port_clone_conn(supervisor:getPort("events"))

dofile("../postamble.lua")
