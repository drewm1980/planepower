#!/usr/bin/python
import casadi
from casadi import ssym, mul, jacobian, \
			sin,cos,arctan2, solve, vertcat,\
			jacobianTimesVector, sqrt
import numpy
import casadi_conveniences
from casadi_conveniences import *
from rotation_matrices import Rx,Ry,Rz,Tx,Ty,Tz,unskew

####################
# This script is for investigating whether several
# carousel-like mechanical systems are open loop stable.
####################

# Note, in this file we use x-y "math" coordinates, rather
# than North-East-Down "aero" coordinates

t = casadi.ssym('t')

R = ssym('R')  # length of carousel arm
r = ssym('r')  # length of tether
ddelta = ssym('ddelta')  # Rotational speed of the arm [rad/s]

# Horizontal azimuth angle of the tether in front of the arm
az = ssym('az') 
daz = ssym('daz')
ddaz = ssym('ddaz')

# generalized coordinate
q  = casadi.vertcat([az])
dq  = casadi.vertcat([daz])
ddq  = casadi.vertcat([ddaz])
nq = q.size()

# some subexpressions
s_q = sin(az)
c_q = cos(az)

# stone position in world coordinates.  Maps az,t -> x
x_w = casadi.vertcat([R+r*c_q, r*s_q])

d = r*(dq + ddelta) # intermedite variable

v_w = casadi.vertcat([0,R*ddelta]) \
        + d*casadi.vertcat([-s_q,c_q])

# Assume we are spinning a 7kg sphere of water
m = 7.0 # kg
Cd = 0.47 # Drag coefficient for a sphere
rho_water = 1000. # kg / m^2
pi = 3.1415
# volume = 4/3 * pi * r*r*r
# m = volume * rho_water
# m/rho_water = 4/3 * pi * r*r*r
r = (m/rho_water*3./4./pi)**(1./3.)
A = pi * r * r
rho_air = 1.2 # kg / m^2
# Constant in front of the v^2
CD = 0.5*Cd*A*rho_air

# Aero. force in world frame as function of az,t
vdotv = R*R*ddelta*ddelta + d*d + d*R*ddelta*c_q
speed = sqrt(vdotv)
f_w = -CD*speed*v_w

# Generalized aerodynamic force
dxdq=casadi.vertcat([r*-s_q, r*c_q])
f_gen = mul(dxdq.T,f_w)

# Formulate the Lagrangian
KE = 0.5*m*vdotv
PE = 0

####################
# Derive the implicit ODE (DAE-like) form of the equations of motion
####################
L = KE-PE

dLdq = jacobian(L,q) # d/d(q) L.  Contains q,dq. 1xlen(q)
dLddq = jacobian(L,dq) # d/d(dq) L.  Contains q,dq. 1xlen(q)

q_and_dq = q[:]
q_and_dq.append(dq) # [q; dq]
dq_and_ddq = dq[:]
dq_and_ddq.append(ddq) # [dq; ddq]

#LHS1 = total_derivative(dLddq,q_and_dq,dq_and_ddq) # d/dt dL/d(dq)
J_temp = jacobian(dLddq, q_and_dq)
LHS1_ = mul(J_temp,dq_and_ddq).reshape(dLddq.shape)
LHS1 = jacobianTimesVector(dLddq.T,q_and_dq,dq_and_ddq).reshape(dLddq.shape)

LHS = (LHS1 - dLdq).T  
RHS = f_gen 

# Dynamics are satisfied when this vector expression is zero.
DAE_RHS = LHS-RHS

####################
# Derive the ODE form of the equations of motion
####################

# Working here, need to make sure coriolis terms don't screw this up:

# c + M * ddq - dLddq = generalized_forces
c_ = mul(J_temp[:,:nq],dq).reshape(dLddq.shape)
c = jacobianTimesVector(dLddq.T, q, dq).reshape(dLddq.shape)
M = J_temp[:,nq:] # Mass matrix, i.e. the term that got multiplied by ddq

# M*ddq = RHS
RHS = f_gen + dLdq.T - c.T

RHS2 = solve(M,RHS) # ddq = RHS2 Symbolic system solve.

ODE_RHS = vertcat([dq,RHS2]) # d[q;dq]/dt = [dq; RHS2] 

####################
# Function for setpoint computation.
####################


####################
# Linearize the system about a setpoint and determine stability
####################

####################
# Debugging and testing stuff...
####################

####################
if __name__=='__main__':
	cn = casadi.countNodes
	print "J_temp has %i nodes"%cn(J_temp)
	#print "M has %i nodes"%cn(M)
	print "c_ has %i nodes"%cn(c_)
	print "c has %i nodes"%cn(c)
	print "dLddq has %i nodes"%cn(dLddq)
	print "LHS1_ has %i nodes"%cn(LHS1_)
	print "LHS1 has %i nodes"%cn(LHS1)
	print "DAE_RHS has %i nodes"%cn(DAE_RHS)

	print "ODE_RHS has %i nodes"%cn(ODE_RHS)

	
