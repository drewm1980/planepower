clear all
close all
clc
numStates = 22; %state = [x;y;z;dx;dy;dz;q0;q1;q2;q3;w1;w2;w3;r;dr;delta;ddelta]
numCtrl   = 4;   %[dddelta(carousel accelration), ddr(cable acceleration), ailerons, elevator]
t = 0;
t_s = 0.05;
%Pertubation of state and control vectors
xpert = 0.001*ones(numStates,1);
upert = [0.001, 0.001, 0.001, 0.001];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%Equilibrium point
x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
% x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
u=[-0.000000000000000000e+00 -0.000000000000000000e+00 -0.00042919 -0.01271043];
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
sys_d = c2d(sys,0.05);
 sim('c2d_check_simmdl')
load out_c 
load out_d
x_c = out_c.Data;
t_c  =out_c.Time;
x_d = out_d.Data;
t_d  =out_d.Time;
figure(1)

 subplot(3,1,1)
plot(t_c,x_c(:,1),'--r'); 
hold on
plot(t_d,x_d(:,1),'.'); 
title('positions')
 subplot(3,1,2)
plot(t_c,x_c(:,2),'--r'); 
hold on
plot(t_d,x_d(:,2),'.'); 
 subplot(3,1,3)
plot(t_c,x_c(:,3),'--r'); 
hold on
plot(t_d,x_d(:,3),'.');
%%%%% Velocities
figure(2)

 subplot(3,1,1)
plot(t_c,x_c(:,4),'--r'); 
hold on
plot(t_d,x_d(:,4),'.'); 
title('velocities')
 subplot(3,1,2)
plot(t_c,x_c(:,5),'--r'); 
hold on
plot(t_d,x_d(:,5),'.'); 
 subplot(3,1,3)
plot(t_c,x_c(:,6),'--r'); 
hold on
plot(t_d,x_d(:,6),'.');
%%%%% rotation matrix
figure(3)

 subplot(3,3,1)
plot(t_c,x_c(:,7),'--r'); 
hold on
plot(t_d,x_d(:,7),'.'); 
title('Rotation')
 
 subplot(3,3,2)
plot(t_c,x_c(:,8),'--r'); 
hold on
plot(t_d,x_d(:,8),'.'); 

 subplot(3,3,3)
plot(t_c,x_c(:,9),'--r'); 
hold on
plot(t_d,x_d(:,9),'.'); 

 subplot(3,3,4)
plot(t_c,x_c(:,10),'--r'); 
hold on
plot(t_d,x_d(:,10),'.'); 

 subplot(3,3,5)
plot(t_c,x_c(:,11),'--r'); 
hold on
plot(t_d,x_d(:,11),'.'); 

 subplot(3,3,6)
plot(t_c,x_c(:,12),'--r'); 
hold on
plot(t_d,x_d(:,12),'.'); 

 subplot(3,3,7)
plot(t_c,x_c(:,13),'--r'); 
hold on
plot(t_d,x_d(:,13),'.'); 
 subplot(3,3,8)
plot(t_c,x_c(:,14),'--r'); 
hold on
plot(t_d,x_d(:,14),'.'); 

 subplot(3,3,9)
plot(t_c,x_c(:,15),'--r'); 
hold on
plot(t_d,x_d(:,15),'.'); 
%%%%% Angular  Velocities
figure(4)

 subplot(3,1,1)
plot(t_c,x_c(:,16),'--r'); 
hold on
plot(t_d,x_d(:,16),'.'); 
title('angular velocities')
 subplot(3,1,2)
plot(t_c,x_c(:,17),'--r'); 
hold on
plot(t_d,x_d(:,17),'.'); 
 subplot(3,1,3)
plot(t_c,x_c(:,18),'--r'); 
hold on
plot(t_d,x_d(:,18),'.');

%%%%% tether position and tether velocity
figure(5)

 subplot(2,1,1)
plot(t_c,x_c(:,19),'--r'); 
hold on
plot(t_d,x_d(:,19),'.'); 
title('tether length and tether_velocity')

 subplot(2,1,2)
plot(t_c,x_c(:,20),'--r'); 
hold on
plot(t_d,x_d(:,20),'.'); 


%%%%% Carousel angular position and angular velocity
figure(6)

 subplot(2,1,1)
plot(t_c,x_c(:,21),'--r'); 
hold on
plot(t_d,x_d(:,21),'.'); 
title('Carousel angular position and angular velocity')

 subplot(2,1,2)
plot(t_c,x_c(:,22),'--r'); 
hold on
plot(t_d,x_d(:,22),'.'); 

