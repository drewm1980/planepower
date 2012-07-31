clear all
close all
clc
numStates = 22; %state = [x;y;z;dx;dy;dz;q0;q1;q2;q3;w1;w2;w3;r;dr;delta;ddelta]
numCtrl   = 4;   %[dddelta(carousel accelration), ddr(cable acceleration), ailerons, elevator]
t = 0;
dt = 0.05;       %
Tf = 10;
nPH = 20;
nCH = 19;
lb = [-2;-2;-1;-1];
ub= [2;2;1;1];

%Pertubation of state and control vectors
xpert = 0.001*ones(numStates,1);
upert = [0.001, 0.001, 0.001, 0.001];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Compute a new equilibrium
z = 0.0;
r = 1.18;
delta = 0;
RPM = 30;
ddelta = 2*pi*RPM/60.;

Nref = 1;
!rm eq/eqIG.txt
fid = fopen(['eq/eqIG.txt'], 'w');
IG = [z r delta ddelta dt Nref];
fprintf(fid,'%6.16e %6.16e %6.16e %6.16e %6.16e %6.16e \n',IG');
fclose(fid);

[Tref,Xref,Uref] = generate_referenceACADO(z,r,delta,ddelta,dt,Nref);
% P.u = [0,0,Xref(1,end-1:end)];
% Model_integ_ACADO_rotation_m_discrete(t,Xref(1,1:end-2),'',P)
% keyboard

%Equilibrium point
x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
x = Xref(1:end-2);
% x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
u=[-0.000000000000000000e+00 -0.000000000000000000e+00 -0.00042919 -0.01271043];
u = [Uref(1:2), Xref(1,end-1:end)];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Linearisation using central difference
[Apos, Bpos]=linearize_AC_rotation(t,x,u,xpert,upert,numStates,numCtrl);
[Aneg, Bneg]=linearize_AC_rotation(t,x,u,-xpert,-upert,numStates,numCtrl);
A = (Apos+Aneg)/2;
B = (Bpos+Bneg)/2;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%matrix condensing
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

C = eye(numStates,numStates);
D = zeros(numStates,numCtrl);
sys = ss(A,B,C,D);
 [A,B,C] = minlin(A,B,C);
 sysd = c2d(sys,dt);
Q =5*eye(length(A),length(A));
R = 5*eye(length(u),length(u));

 [K,P,E]=dlqr(sysd.a,sysd.b,Q,R);
[ H,F1,F2,F3,G,E ]=mpc_condensing( sysd.a,sysd.b,C,D,nPH,nCH,Q,R,P );


% % creating reference trajectory up to prediction horizon
% x_ref = x';    
% x_ref(4) = 0;x_ref(5) = 0;
% y_ref = x_ref;
% for i=2:nPH
%     y_ref = [y_ref;x_ref];
% end

% creating vector of trim controls up to control horizon
u0 = u';   
u_ref  =u0;
for i=2:nCH
   u0 = [u0;u_ref];
end

% creating vector upper and lower controls bound up to control horizon
 
lb_ref  =lb;
ub_ref  =ub;
for i=2:nCH
   lb = [lb;lb_ref];
   ub = [ub;ub_ref];
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%MPC in a closed loop
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


xr = x';


intoptions.AbsTol = 1e-4;
intoptions.RelTol = 1e-4;
intoptions.MaxStep = 1/10;
intoptions.Events = 'off';

statek  = x;state = x;time = 0;tu = [];
while (time(end) < Tf)
 
    X0 = rotation_carousel(statek(end,:)')+ rand(22,1).*[ones(22,1)]*0.01;%added perturbation 
    xr(4) = 0;xr(5) = 0;X0(21) = 0;
    dX0 = X0-xr;
    g = F1*dX0;%+F2*y_ref;
    [du,fval,exitflag,iter,lambda] = qpOASES( H,g,lb,ub);
%     du = quadprog(H,g',[],[],[],[],lb,ub);
    u = du+u0;
    uk =u(1:numCtrl);
    P.u = uk;
  
    [timek,statek]=ode45('Model_integ_ACADO_rotation_m_discrete',[time(end) time(end)+dt],statek(end,:)',intoptions,P);
    state = [state;statek];
    time = [time;timek];
    tu = [tu [time(end);uk]];
    display(['Simulation: ',num2str(100*time(end)/Tf),' %']);clc
    
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%MPC in closed loop
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

figure(1)
plot3(state(:,1),state(:,2),state(:,3))
axis equal
grid on
xlabel('x')
ylabel('y')
zlabel('z')
figure(2);
plot(state(:,1),state(:,2));
axis equal

figure(3)
for k = 1:4
    subplot(2,2,k)
    stairs(tu(1,:),tu(k+1,:))
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%Discrete LQR in closed loop
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Equilibrium point
x = Xref(1:end-2);

u = [Uref(1:2), Xref(1,end-1:end)];

xr = x';
u0 = u;

intoptions.AbsTol = 1e-4;
intoptions.RelTol = 1e-4;
intoptions.MaxStep = 1/10;
intoptions.Events = 'off';

statek  = x;state = x;time = 0;tu = [];
while (time(end) < Tf)
 
    X0 = rotation_carousel(statek(end,:)')+ rand(22,1).*[ones(22,1)]*0.01;%added perturbation 
    xr(4) = 0;xr(5) = 0;X0(21) = 0;
    dX0 = X0-xr;
    uk = -K*(dX0)+u0';
    P.u = uk;
  
    [timek,statek]=ode45('Model_integ_ACADO_rotation_m_discrete',[time(end) time(end)+dt],statek(end,:)',intoptions,P);
    state = [state;statek];
    time = [time;timek];
    tu = [tu [time(end);uk]];
    display(['Simulation: ',num2str(100*time(end)/Tf),' %']);clc
    
end


figure(1)
hold on;
plot3(state(:,1),state(:,2),state(:,3),'-r')
axis equal
grid on
xlabel('x')
ylabel('y')
zlabel('z')
figure(2)
hold on
plot(state(:,1),state(:,2),'-r');
axis equal

figure(3)
hold on
for k = 1:4
    subplot(2,2,k)
    hold on
    stairs(tu(1,:),tu(k+1,:),'-r')
end




















