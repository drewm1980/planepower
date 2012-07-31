clear all
close all
clc
numStates = 17; %state = [x;y;z;dx;dy;dz;q0;q1;q2;q3;w1;w2;w3;r;dr;delta;ddelta]
numCtrl   = 4;   %[dddelta(carousel accelration), ddr(cable acceleration), ailerons, elevator]
t = 0;
%Pertubation of state and control vector
xpert = 0.001*ones(numStates,1);
upert = [0.001, 0.001, 0.001, 0.001];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%Equilibrium point
x = [0.082717446096388231, 2.262097202490879, 0.0, -7.1065879530522071, 0.25986452098049251, 0.0, 0.055249643638419048, 0.013696508823130116, 0.59389645691928372, 0.80252531485466072, -0.13710395745772588, 2.9994272187729876, 0.92425281456218544, 1.1799999999999999, 0.0, 0.0, 3.1415926535897927];
u=[-0.000000000000000000e+00 -0.000000000000000000e+00 -0.00042919 -0.01271043];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Linearisation using central difference
[Apos, Bpos]=linearize_AC(t,x,u,xpert,upert,numStates,numCtrl);
[Aneg, Bneg]=linearize_AC(t,x,u,-xpert,-upert,numStates,numCtrl);
A = (Apos+Aneg)/2;
B = (Bpos+Bneg)/2;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


C = eye(numStates,numStates);
D = zeros(numStates,numCtrl);
sys = ss(A,B,C,D);
[A,B,C] = minlin(A,B,C);

