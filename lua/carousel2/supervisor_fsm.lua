return rfsm.state {
   stopped = rfsm.state {
      entry=function() print("Entered stopped state") end,
   },
 
   rotating = rfsm.state {
      entry=function() print("Entered rotating state") end,
   },
 
   rfsm.trans {src="initial", tgt="stopped" },
   rfsm.trans {src="stopped", tgt="rotating", events={"e_start"}},
   rfsm.trans {src="rotating", tgt="stopped", events={"e_stop"}},
}
