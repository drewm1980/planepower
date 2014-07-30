clear;
reset(symengine);

%ground frame: NED centered at the carousel axis at arm level
syms t %time [s]
syms p_ball(t) %position of the ball in ground frame, [m]
syms r_arm %length of arm [m]
syms l_tether %length of tether [m]
syms m_ball %mass of the ball [kg]
syms I_arm %moment of inertia of carousel [kg m^2]
syms I_motor %moment of inertia of motor [kg m^2]
syms p_sensor(t) %position of line angle sensor in ground frame [m]
syms delta_arm(t) %angle of arm rotating around carousel axis with 0 aligned with east, positive for counter-clockwise rotation [rad]
syms delta_motor(t) %angle of motor rotating around carousel axis with 0 aligned with east, positive for counter-clockwise rotation [rad]
syms alpha(t) %elevation angle of rope, positive above horizontal [rad]
syms beta(t) %azimuth angle of rope, positive if ball is ahead of the arm [rad]
syms k %spring constant of the belt [Nm/rad]
syms g %gravitational constant

%defining geometric subexpressions
p_sensor = r_arm*[sin(delta_arm);cos(delta_arm);0];
p_ball = p_sensor+l_tether*[cos(alpha)*sin(delta_arm+beta);cos(alpha)*cos(delta_arm+beta);-sin(alpha)];

%kinetic energy
v_ball = diff(p_ball,t);
KE = 0;
KE = KE + 1/2*m_ball*(v_ball.'*v_ball);
KE = KE + 1/2*I_arm*diff(delta_arm,t)^2;
KE = KE + 1/2*I_motor*diff(delta_motor,t)^2

%potential energy
PE = m_ball*g*(-l_tether*sin(alpha));
PE = PE +1/2*k*(delta_motor-delta_arm)^2

%lagrangian
L = KE - PE

%substitute in uniform names for our generalized coordinates
syms q1(t) q2(t) q3(t) q4(t)
syms q1dot(t) q2dot(t) q3dot(t) q4dot(t)
syms q1ddot(t) q2ddot(t) q3ddot(t) q4ddot(t)
vars = [delta_arm, delta_motor, alpha, beta];
diffvars = [diff(delta_arm,t),diff(delta_motor,t),diff(alpha,t),diff(beta,t)];
diffqs = [diff(q1,t),diff(q2,t), diff(q3,t), diff(q4,t)];
diffqdots = [diff(q1dot,t), diff(q2dot,t), diff(q3dot,t), diff(q4dot,t)];
qs = [q1, q2, q3,q4];
qdots = [q1dot, q2dot, q3dot, q4dot];
qddots= [q1ddot, q2ddot, q3ddot, q4ddot];
L = subs(L, vars, qs);
L = subs(L, diffvars, qdots);
L = subs(L, diffqs, qdots);

% Compute residual of the Euler-Lagrange equation:
% 0 = d/dt dL/dqdot - dL/dq
% Note, the diffDepVar makes it possible to differentiate f(g(t)) with 
% respect to g, without applying chain rule on g.
eq1 = diff(diffDepVar(L,q1dot), t) - diffDepVar(L,q1)
eq2 = diff(diffDepVar(L,q2dot), t) - diffDepVar(L,q2)
eq3 = diff(diffDepVar(L,q3dot), t) - diffDepVar(L,q3)
eq4 = diff(diffDepVar(L,q3dot), t) - diffDepVar(L,q4)

% The above manipulation introduced new and old derivative terms,
% notably acceleration terms.
% Rename them for uniformity.

eqs = vertcat(eq1, eq2, eq3, eq4);
eqs = subs(eqs,diffqs,qdots)
eqs = subs(eqs,diffqdots,qddots)

% The time dependence isn't relevant to solving the above equations for q*ddot
syms         Q1 Q2 Q3 Q4 Q1dot Q2dot Q3dot Q4dot Q1ddot Q2ddot Q3ddot Q4ddot;
Qs = [Q1,Q2,Q3,Q4];
Qdots = [Q1dot,Q2dot,Q3dot,Q4dot];
Qddots = [Q1ddot,Q2ddot,Q3ddot,Q4ddot];
eqs = subs(eqs
eq1 = subs(eq1,subslist3,subslist4)
eq2 = subs(eq2,subslist3,subslist4)
eq3 = subs(eq3,subslist3,subslist4)
eq4 = subs(eq4,subslist3,subslist4)

% Expanding terms will reduce the number of parantheses, making
% it easier to see what is going on...
eq1 = expand(eq1)
eq2 = expand(eq2)
eq3 = expand(eq3)
eq4 = expand(eq4)

% put all second derivatives and equations into vectors
% since second derivatives enter multiplicatively, M corresponds to their
% coefficients
% eqs = M*Qddot + c


Qddot = vertcat(Q1ddot, Q2ddot, Q3ddot, Q4ddot);

M = jacobian(eqs,Qddot);
c = simplify(eqs - M*Qddot);

% Define our full state vector for our ODE
state = [Q1; Q2; Q3; Q4; Q1dot; Q2dot; Q3dot; Q4dot];
% first part of f(x,u)
subs(M,state,zeros(size(state)))
f1 = [Q1dot; Q2dot; Q3dot; Q4dot];
f2 = M\(-c);

f = [f1; f2]
