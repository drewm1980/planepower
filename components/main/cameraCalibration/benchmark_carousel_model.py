#!/usr/bin/env python
# Benchmark my casadi-based dynamic equations

import casadi
from carousel_model import DAE_RHS, ODE_RHS, q, dq, ddq, constants, inputs

# Specialize dynamic equations for the constants (modeling parameters)
eqns = casadi.substitute(ODE_RHS,constants.keys(),constants.values())

ins = casadi.vertcat([q,dq,inputs])
f = casadi.SXFunction([ins],[eqns])
f.init()
f.generateCode('carousel_model.c')

#from cse import cse
#from carousel_model import ODE_RHS as f

#cn = casadi.countNodes
#f2 = cse(f)
#print cn(f)
#print cn(f2)

