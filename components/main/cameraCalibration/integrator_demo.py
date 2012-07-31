#!/usr/bin/env python
import casadi
from casadi import *

# Spring-mass model.
m = casadi.ssym('m') # Mass
k = casadi.ssym('k') # Spring constant
x0 = casadi.ssym('x0') # Relaxed position of spring
w = casadi.ssym('w') # Frequency of the input force
A = casadi.ssym('A') # Amplitude of the input force

t = casadi.ssym('t') # Time
x = casadi.ssym('x') # Position of the mass
dx = casadi.ssym('dx') # Velocity of the mass
#ddx = casadi.ssym('ddx') # Acceleration of the mass

# Input force
u = casadi.ssym('u')
u_ref = A * casadi.cos(w*t)
u_dev = u-u_ref
u_dev *= u_dev

# DAE form of Newton's equation
dynamic_equations = m*ddx - (-k*(x-x0) + u)

# State and derivative of the state
q = casadi.vertcat([x,dx])
dq = casadi.vertcat([dx,ddx])

# Input of the DAE residual function
ffcn_in = makeVector(SXMatrix,DAE_NUM_IN,DAE_T,t,DAE_X,q,DAE_XDOT,dq,DAE_P,u)
ffcn_out = makeVector(SXMatrix,DAE_NUM_OUT,DAE_ODE,dynamic_equations,DAE_QUAD,u_dev)

f = SXFunction(ffcn_in, ffcn_out)
