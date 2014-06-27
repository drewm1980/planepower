clear all
close all
clc
numStates = 22; %state = [x;y;z;dx;dy;dz;q0;q1;q2;q3;w1;w2;w3;r;dr;delta;ddelta]
numCtrl   = 4;   %[ddr(cable acceleration),dddelta(carousel accelration), ailerons, elevator]
t = 0;
dt = 0.4;       %
Tf = 100;
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
% x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
x = Xref(1:end-2);
% x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
% u=[-0.000000000000000000e+00 -0.000000000000000000e+00 -0.00042919 -0.01271043];
u = [Uref(1:2), Xref(1,end-1:end)];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
P.delta = x(1,21);
P.ddelta = x(1,22);
P.dddelta = u(1); 
P.u      = u;
P.W0 = 0;
%[A,B,~,~] = jacobianI(x,P);  % Linearize the system at the equilibrium
[A,B,~,~] = jacobian(x,P);  % Linearize the system at the equilibrium

We = 100;
R = 1e-2*diag([100, 100, 2500, 2500]);
Q = 1e-2*diag([25,  25,  25, 1, 1, 1,    ...
                We,  We,  We,  We,  We,  We,  We,  We,  We, ...
                1, 1, 1,   1.00000000e+02,   1.11111111e-01, 100, 100]);

% Compute the terminal cost for the LQR
CJ = invariantJac(x);
CJ = orth(CJ');
J = null(CJ');
[Kred,Sred,ered] = lqr(J.'*A*J,J.'*B,J.'*Q*J,R);
K = Kred*J.';
S = J*Sred*J.';

% eig(A-B*K)
% eig(J.'*A*J-J.'*B*Kred)

Q = J*(J.'*Q*J)*J.';

We = 100;
R = 1e-2*diag([2500, 2500]);
Q = 1e-2*diag([25,  25,  25, 1, 1, 1,    ...
                We,  We,  We,  We,  We,  We,  We,  We,  We, ...
                1, 1, 1, ]);



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%Linearisation using central difference
[Apos, Bpos]=linearize_AC_rotation(t,x,u,xpert,upert,numStates,numCtrl);
[Aneg, Bneg]=linearize_AC_rotation(t,x,u,-xpert,-upert,numStates,numCtrl);
A = (Apos+Aneg)/2;
B = (Bpos+Bneg)/2;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Removing the  columns corresponding to line acceleration and line
%acceleration
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
B = B(:,3:4);
numCtrl = 2;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%Discrete LQR
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

C = eye(numStates,numStates);
D = zeros(numStates,numCtrl);

 [A,B,C] = minlin(A,B,C);
 sys = ss(A,B,C,D);
 sysd = c2d(sys,dt);
% Q =5*eye(length(A),length(A));
% R = 5*eye(numCtrl,numCtrl);
[K,S,E] = dlqr(sysd.a,sysd.b,Q,R);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%Discrete LQR in closed loop
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


xr = x';
u0 = u(3:4);

intoptions.AbsTol = 1e-4;
intoptions.RelTol = 1e-4;
intoptions.MaxStep = 1/10;
intoptions.Events = 'off';

statek  = x;state = x;time = 0;tu = [];
while (time(end) < Tf)
 
    X0 = rotation_carousel(statek(end,:)')+ rand(22,1).*ones(22,1)*0.1;%added perturbation 
    xr(4) = 0;xr(5) = 0;X0(21) = 0;
    dX0 = X0-xr;
    dX0 = dX0(1:end-4);
    uk = -K*(dX0)+u0';
    uk = [0;0;uk];
    P.u = uk;
  
    [timek,statek]=ode45('Model_integ_ACADO_rotation_m_discrete',[time(end) time(end)+dt],statek(end,:)',intoptions,P);
    state = [state;statek];
    time = [time;timek];
    tu = [tu [time(end);uk]];
    display(['Simulation: ',num2str(100*time(end)/Tf),' %']);clc
    
end


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

