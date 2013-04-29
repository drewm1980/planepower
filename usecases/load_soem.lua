dofile("preamble.lua")

--deployer:import("libsoem_ebox-gnulinux.so")
--deployer:import("librtt-soem_ebox-typekit-gnulinux.so")
--deployer:import("libsoem_master-gnulinux.so")
--deployer:import("libsoem_driver_factory-gnulinux.so")
--deployer:import("libsoem_core.so")
--deployer:import("librtt-soem_ebox-ros-transport-gnulinux.so")
--deployer:import("libsoem_plugin-gnulinux.so")

--deployer:loadComponent("soem_master")
--deployer:loadComponent("soem_ebox")
deployer:import("soem_master")
deployer:import("soem_ebox")
deployer:loadComponent("soemMaster","soem_master::SoemMasterComponent")
--soemMaster.ifname = "eth1"

dofile("postamble.lua")
