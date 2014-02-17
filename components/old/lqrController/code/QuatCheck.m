clear all

syms q0 q1 q2 q3 real
syms w1 w2 w3 real
syms x y z r real

E = [-q1  q0 -q3  q2;
     -q2  q3  q0 -q1;
     -q3 -q2  q1  q0];
 
G = [-q1  q0  q3 -q2;
     -q2 -q3  q0  q1;
     -q3  q2 -q1  q0];
 
R = E*G';

    er(1) = x/r;
	er(2) = y/r;
	er(3) = z/r;

dn = [w1 w2 w3]*R';%*er'