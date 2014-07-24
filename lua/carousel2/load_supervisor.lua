#!/usr/bin/env rttlua-i

deployer:loadComponent("supervisor", "OCL::LuaComponent")
supervisor = deployer:getPeer("supervisor")
supervisor:exec_file("supervisor_fsm_init.lua")
supervisor:configure()

write_event = rttlib.port_clone_conn(supervisor:getPort("events")):write

supervisor:start()

--write_event("e_ping")
--write_event("e_pong")

