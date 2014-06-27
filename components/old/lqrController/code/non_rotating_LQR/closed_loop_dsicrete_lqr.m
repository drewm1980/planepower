clear all
close all

clc
numStates = 22; %state = [x;y;z;dx;dy;dz;e11;e12;e13;e21;e22;e23;e31;e32;e33;w1;w2;w3;r;dr;delta;ddelta]               0.1 sec/60degree @6.0V RX-565CBE Power Micro  Eco Digital--2.2Kg/cm
numCtrl   = 3;   %[dddelta(carousel accelration), ddr(cable acceleration), ailerons, elevator]                         0.12 sec/60degree @4.8V RX-565CBE Power Micro  Eco Digital---1.9Kg/cm
t = 0;
dt = 0.1;       %
Tf = 10;
%Pertubation of state and control vectors
xpert =1e-6*ones(numStates,1);
upert = [1e-3,1e-3, 1e-3];
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

MPC.Ref.z=z;
MPC.Ref.r=r;
MPC.Ref.delta=delta;
MPC.Ref.delta0=delta;
MPC.Ref.ddelta=ddelta;
MPC.Ts=dt;
MPC.Nref=Nref;


MPC = generate_reference(MPC);
 
Tref = MPC.Tref;
Xref = MPC.Xref;
Uref = MPC.Uref;


%Equilibrium point
% x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
x = Xref;%[Xref(1:18) Xref(21:22)];
P.delta = Xref(19);
P.ddelta = Xref(20);
% x =[ 0.08271745,  2.2620972 ,  0.        , -7.10658795,  0.25986452,   0.        , -0.99351977, -0.07240986,  0.08760873,  0.10494709,   -0.28846895,  0.95172043, -0.04364155,  0.95474734,  0.29419881,    -0.13710396,  2.99942722,  0.92425281,  1.18      ,  0.        ,        0.        ,  3.14159265];
% u=[-0.000000000000000000e+00 -0.000000000000000000e+00 -0.00042919 -0.01271043];
u = Uref(2:3);
P.dddelta = Uref(1);


% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% P.delta = x(1,21);
% P.ddelta = x(1,22);
% P.dddelta = u(1); 
P.u      = u;
P.tu  = [0 u];
P.r = 1.2;
P.W0 = 0;
% keyboard
%[A,B,~,~] = jacobianI(x,P);  % Linearize the system at the equilibrium
[A,B,~,~] = jacobianI(x,P);  % Linearize the system at the equilibrium
A = A([1:18,21:22],:);
B = B([1:18,21:22],:);

We = 100;
R = 1e-4*diag([100, 100]);
Q = 1e-2*diag([100,  100,  100, 1, 1, 1,    ...
                We,  We,  We,  We,  We,  We,  We,  We,  We, ...
                1, 1, 1, 2500, 2500]);


            

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Linearisation using central difference
% [Apos, Bpos]=linearize_AC_rotation(t,x,u,xpert,upert,numStates,numCtrl);
% [Aneg, Bneg]=linearize_AC_rotation(t,x,u,-xpert,-upert,numStates,numCtrl);
% A1 = (Apos+Aneg)/2;
% B1 = (Bpos+Bneg)/2;  

% Compute the terminal cost for the LQR
% CJ = invariantJac(x);
% CJ = orth(CJ');
% J = null(CJ');
C = eye(numStates-2,numStates-2);
D = zeros(numStates-2,numCtrl-1);
% 
%[A,B,C] = minlin(A1,B1,C);
 sys = ss(A,B,C,D);
 sysd = c2d(sys,dt);
[K,S,E] = dlqr(sysd.a,sysd.b,Q,R);
% K = Kred*J.';
% S = J*Sred*J.';

% eig(A-B*K)
% eig(J.'*A*J-J.'*B*Kred)

% Q = J*(J.'*Q*J)*J.';





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%Discrete LQR
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% C = eye(numStates,numStates);
% D = zeros(numStates,numCtrl);
% % 
% %  [A,B,C] = minlin(A,B,C);
%  sys = ss(A1,B1,C,D);
%  sysd = c2d(sys,dt);
% % Q =5*eye(length(A),length(A));
% % R = 5*eye(length(u),length(u));
% [K,S,E] = dlqr(sysd.a,sysd.b,Q,R);
% K = K*J.';

% dlmwrite('K.dat',K, 'precision','%10.10f');
% dlmwrite('x_ref.dat',x, 'precision','%10.10f');
% dlmwrite('u0.dat',u,'precision','%10.10f');


     dlmwrite('/home/hammad/PLANEPOWER/TRUNK/orocos/components/main/lqrController/src/code/K.dat', K, 'delimiter', '\t', ...
         'precision', 6);
     dlmwrite('/home/hammad/PLANEPOWER/TRUNK/orocos/components/main/lqrController/src/code/x_ref.dat', x, 'delimiter', '\t', ...
         'precision', 6);
     dlmwrite('/home/hammad/PLANEPOWER/TRUNK/orocos/components/main/lqrController/src/code/u0.dat', u, 'delimiter', '\t', ...
         'precision', 6);
     


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%
%Discrete LQR in closed loop
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


u0 = u;

intoptions.AbsTol = 1e-4;
intoptions.RelTol = 1e-4;
intoptions.MaxStep = 1/10;
intoptions.Events = 'off';

state = x;time = 0;tu = [];
xr = x;
% state(1)=1;
% state(2)=sqrt(r^2-state(1)^2-state(3)^2);
P.u = u;
P.xr = xr;

P.K = K;

while time(end) < Tf
  
dX0 = state(end,:)-xr;
dX0(19)=0;
dX0(20)=0;
xr

dX0
% dX0(19) = 0;
uk = -0*P.K*(dX0')


P.uk = uk;
    [timek,statek]=ode45('Model_integ_ACADO_new',[time(end) time(end)+dt],state(end,:)',intoptions,P);
    state = [state;statek];
    time = [time;timek];

%     tu = [tu [time(end) uk]];
    display(['Simulation: ',num2str(100*time(end)/Tf),' %']);
% pause
 
end


figure(1)

plot3(state(:,1),state(:,2),state(:,3),'-r')
axis equal
grid on
xlabel('x')
ylabel('y')
zlabel('z')
axis equal

figure(2)

plot(state(:,1),state(:,2),'-r');
axis equal

% figure(3)
% 
% for k = 1:4
%     subplot(2,2,k)
%     
%     stairs(tu(1,:),tu(k+1,:),'-r')
% end


% figure(4)
% 
% for k = 1:4
%     subplot(2,2,k)
%     
%     stairs(diff(tu(1,:))/dt,'-k')
% end