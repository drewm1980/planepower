clear all
close all
clc
numStates = 22; %state = [x;y;z;dx;dy;dz;q0;q1;q2;q3;w1;w2;w3;r;dr;delta;ddelta]
numCtrl   = 4;   %[dddelta(carousel accelration), ddr(cable acceleration), ailerons, elevator]
dt = 0.001; 
t=0;%
Tf = 10;
%Pertubation of state and control vectors
xpert = 0.001*ones(numStates,1);
upert = [0.001, 0.001, 0.001, 0.001];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Compute a new equilibrium
z = 0.0;
r = 1.18;
delta = 0;
RPM = 60;
ddelta = 2*pi*RPM/60.;

Nref = 1;
!rm eq/eqIG.txt
fid = fopen(['eq/eqIG.txt'], 'w');
IG = [z r delta ddelta dt Nref];
fprintf(fid,'%6.16e %6.16e %6.16e %6.16e %6.16e %6.16e \n',IG');
fclose(fid);

[Tref,Xref,Uref] = generate_referenceACADO(z,r,delta,ddelta,dt,Nref);

% keyboard

%Equilibrium point
x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
x = Xref(1:end-2);
% x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
u=[-0.000000000000000000e+00 -0.000000000000000000e+00 -0.00042919 -0.01271043];
u = Uref;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Linearisation using central difference
[Apos, Bpos]=linearize_AC_rotation(t,x,u,xpert,upert,numStates,numCtrl);
[Aneg, Bneg]=linearize_AC_rotation(t,x,u,-xpert,-upert,numStates,numCtrl);
A = (Apos+Aneg)/2;
B = (Bpos+Bneg)/2;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% linmod from matlab

% [A_m,B_m,C_m,D_m]=linmod('sim_simulink_lin',x,u);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

C = eye(numStates,numStates);
D = zeros(numStates,numCtrl);
sys = ss(A,B,C,D);
 [A,B,C] = minlin(A,B,C);
Q =5*eye(length(A),length(A));
R = 5*eye(length(u),length(u));
[K,S,E] = lqr(A,B,Q,R);



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% LQR CONTROLLER
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% K = P.K;
% xr = P.xr;
% ur = P.ur;
% 
% Rd   = [[cos(delta), sin(delta),0];[-sin(delta), cos(delta), 0];[0,0,1]];
% dRd  = [[-sin(delta), cos(delta),0];[-cos(delta), -sin(delta), 0];[0,0,0]]*ddelta;
% % ddRd = [[-sin(delta+pi/2), cos(delta+pi/2),0];[-cos(delta+pi/2), -sin(delta+pi/2), 0];[0,0,0]]*dddelta  ...
% %    +   [[-cos(delta+pi/2), -sin(delta+pi/2),0];[sin(delta+pi/2), -cos(delta+pi/2), 0];[0,0,0]]*ddelta*ddelta;
%     
% 
% X = [x;y;z];
% dX = [dx;dy;dz];
% 
% X0 = Rd*X;
% % dX0 = (dRd*X) + (Rd*dX);
% % ddx = (ddRd*X) + 2*(dRd*dX) + (R*ddX);
%    
% W = [w1;w2;w3];
% W0 = (Rd*W);
% R0 = (Rd*R);
% 
% u1 = ur(3);%3.14788390e-04;
% u2 = ur(4);%1.39979994e-05;
% 
% X = [X0;dX*0;R0(1,:).';R0(2,:).';R0(3,:).';W;r;dr;0.;ddelta;u1;u2];
% 
% % keyboard
% u = -K*(X-xr) + ur;
