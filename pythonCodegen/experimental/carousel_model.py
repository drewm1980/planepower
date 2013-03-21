#!/usr/bin/python
import casadi
from casadi import ssym, mul, jacobian, \
			sin,cos,arctan2, solve, vertcat,\
			jacobianTimesVector
import numpy
import casadi_conveniences
from casadi_conveniences import *
from rotation_matrices import Rx,Ry,Rz,Tx,Ty,Tz,unskew

####################
# This derivation of the kite model starts from scratch with
# the coordinate systems.  This allows more uniformity:
# In the typical flying configuration during rotational
# start, the coordinate systems will be (almost) aligned,
# and all of the angular generalized coordinates will be
# (almost) zero.
####################

t = casadi.ssym('t')

####################
# Physical Constants
####################
g = casadi.SXMatrix([9.80665]) # m/s^2

constants = {}

r_arm = ssym('r_arm') # Radius of the carousel arm [m]
constants[r_arm] = 2 # meters
m_plane = ssym('m_plane') # Mass of the plane
constants[m_plane] = 10 # kg?
I_arm = ssym('I_arm') # Inertia of the carousel arm
constants[I_arm] = 50 # kg-m?

# Inertia tensor of the plane in the plane frame
Ixx = ssym('Ixx')
Iyy = ssym('Iyy')
Izz = ssym('Izz')
Ixy = ssym('Ixy')
I_plane = casadi.SXMatrix(3,3)
I_plane[0,0] = Ixx
I_plane[1,1] = Iyy
I_plane[2,2] = Izz
I_plane[0,1] = Ixy
I_plane[1,0] = Ixy
constants[Ixx] = 10.0
constants[Iyy] = 11.0
constants[Izz] = 12.0
constants[Ixy] = 13.0

# Location of the tether end in plane frame:

# Should probably be comparable to wingspan
# Make more negative for longer bridle, more stability
x_bridle = ssym('x_bridle') 
constants[x_bridle] = -1.0

# Influences natural pitch offset 
y_bridle = ssym('y_bridle') 
constants[y_bridle] = 0.0

# Influences natural yaw offset  
# Make positive to try to get natural flight
# above horizontal during rotational start
z_bridle = ssym('z_bridle')
constants[z_bridle] = 0.0

# Declare inputs
tau_carousel_motor = ssym('tau_carousel_motor')
tau_winch_motor = ssym('tau_winch_motor')
motor_torques = casadi.SXMatrix([tau_carousel_motor,tau_winch_motor])
aileron1 = ssym('aileron1')
aileron2 = ssym('aileron2')
elevator = ssym('elevator')
rudder = ssym('rudder')
control_surfaces = casadi.SXMatrix([aileron1, aileron2, elevator, rudder])
inputs = casadi.vertcat([motor_torques,control_surfaces])

####################
# Generalized coordinates and their time derivatives
# All angles are in radians (like the C standard library)
####################

# The carousel angle, counter-clockwise 
# is positive when viewed from above
delta = ssym('delta') 
ddelta = ssym('ddelta')
dddelta = ssym('ddelta')

# Length of the tether [m]
r_tether = ssym('r_tether') 
dr_tether = ssym('dr_tether')
ddr_tether = ssym('ddr_tether')

# Horizontal lag angle of the tether behind the arm
lag = ssym('lag') 
dlag = ssym('dlag')
ddlag = ssym('ddlag')

# Inclination angle of the tether above horizontal
inc = ssym('inc') 
dinc = ssym('dinc') 
ddinc = ssym('ddinc') 

# Orientation of the plane frame to the tether end frame
roll = ssym('roll')
pitch = ssym('pitch')
yaw = ssym('yaw')
droll = ssym('droll')
dpitch = ssym('dpitch')
dyaw = ssym('dyaw')
ddroll = ssym('ddroll')
ddpitch = ssym('ddpitch')
ddyaw = ssym('ddyaw')

q  = casadi.SXMatrix([ delta,  r_tether,  lag,  inc,  roll,  pitch,  yaw])
dq = casadi.SXMatrix([ddelta, dr_tether, dlag, dinc, droll, dpitch, dyaw])
ddq = casadi.SXMatrix([dddelta, ddr_tether, ddlag, ddinc, ddroll, ddpitch, ddyaw])
nq = q.size()

# Numerical partial state example for debugging:
q_flying = casadi.SXMatrix([ arctan2(1,9),  1.5,   arctan2(1,10),  arctan2(1,11),  0.001,  0.002,  0.003])

####################
# Derive the transformation that maps points in the plane frame
# back to the inertial frame
# Expressed in minimal coordinates
####################

# Maps points in plane arm end frame back to inertial frame
T01 = reduce(mul,[Rz(delta),Tx(r_arm)])

# Maps points in tether end frame back to arm end frame
T12 = reduce(mul,[Ry(-inc), Rz(-lag), Tx(r_tether)])

# Maps points in bridle frame back to arm end frame
T23 = reduce(mul,[Rx(yaw),Ry(roll),Rz(pitch)])

# Maps points in plane frame back to tether end frame
T34 = reduce(mul,[Tx(-x_bridle), Ty(-y_bridle), Tz(-z_bridle)])

# Maps points in plane frame back to inertial frame
T04 = reduce(mul,[T01,T12,T23,T34]) # Function of q

####################
# Compute kinematic and dynamic expressions that will
# be needed in later formulations.
# These expressions should all be independent of the choice of 
# (minimal) generalized coordinates.
####################

J04 = casadi.jacobian(T04, q) # i.e. dT04/dq
dT04 = casadi.mul(J04,dq).reshape(T04.shape)

# Plane center of mass in inertial frame
p0 = T04[:3,3]

# Slice out the rows of J04 corresponding to the p0, i.e. dp0/dq
dp0dq_a = J04[[3,7,11],:] 
# Re-compute dp0/dq in a more simple but less efficient way as a sanity check
dp0dq_b = casadi.jacobian(p0,q) 

def test_dp_equivalence():
	vars = casadi.vertcat((q,constants.keys()))
	vals = casadi.vertcat((q_flying,constants.values()))
	for i in xrange(7):
		assert test_numerically_the_same(dp0dq_a[i],dp0dq_b[i],vars,vals)
		#print [casadi.countNodes(exp[i]) for exp in [dp0dq_a, dp0dq_a]]

test_dp_equivalence()

dp0dq = dp0dq_a

dp0 = dT04[:3,3] # dp/dt, i.e. Velocity of the plane in inertial frame

R04 = T04[:3,:3] # 3x3 SO3 matrix mapping directions in plane frame to inertial frame.
dR04 = dT04[:3,:3] # dR/dt
# I got the following formula for omega from: 
# Robot Dynamics and Control [Spong, Vidyasagar] 1989, page 52, eq. 2.5.13
w0 = unskew(mul(dR04,R04.T)) # Angular velocity vector of plane in inertial frame
w4 = mul(R04.T,w0) # Angular velocity vector of plane in plane frame

####################
# Derive Generalized Forces
# http://en.wikipedia.org/wiki/Generalized_forces
####################

# Given a position in inertial frame and a time,
# return the wind speed vector relative to the ground,
# in inertial frame.
# Wind speed is in units of m/s
def wind_model(p,t):
	wind_ground = casadi.SXMatrix(3,1) # No wind
	wind_ground[0,0] = 1.2 # Add some wind along along x-direction, i.e. delta=0
	return wind_ground

# Return the aerodynamic force and moment about
# the center of mass.
# Input: 3x1 expression for the relative wind in plane frame.
# Note: In the plane coordinate system: 
#		x points "up" from the pilot's perspective
#		y points "forward" from the pilot's perspective
#		z points "left" from the pilot's perspective
# Output: 3x1 expressions for the force and torque, in plane frame
def aerodynamic_model(apparent_wind_plane, w_plane, control_surfaces):

	# Aerodynamic force in (cm-centered) plane frame
	airspeed = casadi.sqrt(mul(apparent_wind_plane.T,apparent_wind_plane))
	translational_viscosity = 8 
	f_plane = translational_viscosity*airspeed*apparent_wind_plane # Translational damping

	# Aerodynamic torque in plane frame
	w_magnitude = casadi.sqrt(mul(w_plane.T,w_plane))
	rotational_viscosity = 11
	tau_plane = -rotational_viscosity*w_magnitude*w_plane # Rotational damping
	return f_plane,tau_plane

apparent_wind_ground = wind_model(p0,t) - dp0 # apparent wind in ground frame
apparent_wind_plane = mul(R04.T,apparent_wind_ground) # apparent wind in plane frame
f_plane,tau_plane = aerodynamic_model(apparent_wind_plane, w4, control_surfaces)

# For generalized force, see http://en.wikipedia.org/wiki/Generalized_force
f_ground = mul(R04,f_plane) # aerodynamic force in ground frame
f_gen = mul(dp0dq.T,f_ground) # generalized aerodynamic force

# For generalized torque I had to derive it myself.
#
# For each joint, an infinitesimal amount of "virtual" work at the joint
# must match the resulting infinitesimal work performed against the real world force.
# Since I only know how to apply a torque vector to an angular velocity vector, I
# start with an equation balancing rates of real work and virtual work.
#
# tau: torque in world coorinates
# omega: angular velocity vector in world coordinates
# taugen: generalized force corresponting to torque
#
# tau' * omega = <taugen,dq/dt>
#
# Expand expression for omega:
# tau' * unskew(dR/dt * R') = taugen' * dq/dt
#
# Expand further
# tau' * unskew(sum_i dR/dq_i * dq_i/dt * R') = taugen' * dq/dt
#
# Only joint i is actually moving...
# tau' * unskew(dR/dq_i * dq_i/dt * R') = taugen_i * dq_i/dt
# tau' * unskew(dR/dq_i * R') = taugen_i
# taugen_i = tau' * unskew(dR/dq_i * R')
# 
tau_ground = mul(R04,tau_plane)
tau_gen = casadi.SXMatrix(nq,1)
for i in xrange(nq):
	dR04dqi = J04[:,i].reshape((4,4))[:3,:3]  # Nothing beats python for
											  # pushing around data structures.
	tau_gen[i,0] = mul(tau_ground.T,unskew(mul(dR04dqi,R04.T)))

# The carousel motor torque acts through the joint corresponding to delta.
tau_gen_motor =  jacobian(q,delta)*tau_carousel_motor

generalized_forces = f_gen + tau_gen + tau_gen_motor

####################
# Formulate the Lagrangian
####################

KE_plane = onehalf * m_plane * mul(dp0.T,dp0) # Translational kinetic energy of the plane
RE_plane = reduce(mul,[onehalf,w4.T,I_plane,w4]) # Rotational kinetic energy of the plane
PE_plane = m_plane * g * p0[2] # Potential energy of the plane
L_plane = KE_plane + RE_plane - PE_plane # The Lagrangian of the plane

# The arm has only rotational kinetic energy, so it's Lagrangian is very simple:
L_arm = onehalf * I_arm * ddelta * ddelta # The Lagrangian of the arm

L = L_arm + L_plane # The Lagrangian of the carousel system.  Contains q,dq

####################
# Derive the implicit ODE (DAE-like) form of the equations of motion
####################

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

LHS = (LHS1 - dLdq).T  # 7x1
RHS = generalized_forces # 7x1

# Dynamics are satisfied when this vector expression is zero.
DAE_RHS = LHS-RHS

####################
# Derive the ODE form of the equations of motion
####################

# c + M * ddq - dLddq = generalized_forces
c_ = mul(J_temp[:,:nq],dq).reshape(dLddq.shape)
c = jacobianTimesVector(dLddq.T, q, dq).reshape(dLddq.shape)
M = J_temp[:,nq:] # Mass matrix, i.e. the term that got multiplied by ddq

# M*ddq = RHS
RHS = generalized_forces + dLdq.T - c.T

RHS2 = solve(M,RHS) # ddq = RHS2 Symbolic system solve.

ODE_RHS = vertcat([dq,RHS2]) # d[q;dq]/dt = [dq; RHS2] 

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

	
