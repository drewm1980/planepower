clc
clear all
close all

clc
numStates = 20; %state = [x;y;z;dx;dy;dz;e11;e12;e13;e21;e22;e23;e31;e32;e33;w1;w2;w3;r;dr;delta;ddelta]               0.1 sec/60degree @6.0V RX-565CBE Power Micro  Eco Digital--2.2Kg/cm
numCtrl   = 2;   %[dddelta(carousel accelration), ddr(cable acceleration), ailerons, elevator]                         0.12 sec/60degree @4.8V RX-565CBE Power Micro  Eco Digital---1.9Kg/cm
t = 0;
dt = 0.1;       %
Tf = 10;
%Pertubation of state and control vectors
xpert =1e-6*ones(numStates,1);
upert = [1e-3,1e-3, 1e-3];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Compute a new equilibrium
% select 1 for u forced to zero and 2 for z forcing the z position

switch 1
          case {1}
              z = 0.0;
              r = 1.2;
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
          case {2}
              
              %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
              %%%Z
              %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
              % Compute a new equilibrium
              z = -0.04;
              r = 1.2;
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
              
              
              MPC = generate_reference_z(MPC);
              
              Tref = MPC.Tref;
              Xref = MPC.Xref;
              Uref = MPC.Uref;       
           
              
           end
 
%X0 = [x;y;z;dx;dy;dz;reshape(R,1,9).';w1;w2;w3;delta;ddelta;ur;up].';

x = Xref(1:20);
u = Xref(21:22);

P.delta = Xref(19);
P.ddelta = Xref(20);
P.dddelta = 0;
P.r = 1.2;
P.W0 = 0;
P.uk = u;

[A,B,~,~] = jacobianI(x,P);  % Linearize the system at the equilibrium
eig(A)
A = A(1:18,1:18);
B = B(1:18,:);
eig(A);


% A = A([1:18],:);
%
%
% B = B([1:18],:);
We = 100;
R = 3*diag([100, 100]);
Q = 1e-2*diag([100,  100,  100, 1, 1, 1,    ...
    We,  We,  We,  We,  We,  We,  We,  We,  We, ...
    1, 1, 1]);

% [Apos, Bpos]=linearize_AC_rotation(t,x,u,xpert,upert,numStates,numCtrl,P);
% [Aneg, Bneg]=linearize_AC_rotation(t,x,u,-xpert,-upert,numStates,numCtrl,P);
% A1 = (Apos+Aneg)/2;
% B1 = (Bpos+Bneg)/2;
% C = eye(numStates,numStates);
% D = zeros(numStates-2,numCtrl);
% %
% [A,B,C] = minlin(A,B,C);

C = eye(numStates-2,numStates-2);
D = zeros(numStates-2,numCtrl);
sys = ss(A,B,C,D);
sysd = c2d(sys,dt);
[K,S,E] = dlqr(sysd.a,sysd.b,Q,R);
Krow = reshape(K',1,[]);
xrow = x(1:18);

dlmwrite('/home/hammad/PLANEPOWER/TRUNK/orocos/components/main/lqrController/src/code/K.dat', Krow, 'delimiter', '\t', ...
    'precision', 6);
dlmwrite('/home/hammad/PLANEPOWER/TRUNK/orocos/components/main/lqrController/src/code/x_ref.dat', xrow, 'delimiter', '\t', ...
    'precision', 6);
dlmwrite('/home/hammad/PLANEPOWER/TRUNK/orocos/components/main/lqrController/src/code/u0.dat', u, 'delimiter', '\t', ...
    'precision', 6);

%closed loop lqr
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
u0 = u;
intoptions.AbsTol = 1e-4;
intoptions.RelTol = 1e-4;
intoptions.MaxStep = 1/10;
intoptions.Events = 'off';
state = x;time = 0;tu = [];input = [];
xr = x;
% state(1)=1;
% state(2)=sqrt(r^2-state(1)^2-state(3)^2);
P.u = u;
P.xr = xr;
while time(end) < Tf
    
    dX0 = state(end,[1:18])-xr(1:18);
    uk = -K*(dX0')+u';
    P.uk=uk;
    [timek,statek]=ode45('Model_integ_ACADO_new',[time(end) time(end)+dt],state(end,:)',intoptions,P);
    state = [state;statek];
    time = [time;timek];
    
    input = [input  uk];
    
    display(['Simulation: ',num2str(100*time(end)/Tf),' %']);
    
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
figure(3)

for k = 1:2
    subplot(2,1,k)
    
    stairs(input(k,:),'-r')
end

figure(4)
stairs(time, state(:,15)) 
% figure(4)
%
% for k = 1:4
%     subplot(2,2,k)
%
%     stairs(diff(tu(1,:))/dt,'-k')
% end