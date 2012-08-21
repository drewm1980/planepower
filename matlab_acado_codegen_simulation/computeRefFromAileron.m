clear all
close all
clc

%Just some random (but feasible) state to start integrating (37 rpm)
x0 =  [1.1254; -0.1911; -0.3700; 0; 0; 0; 0.1257; 0.5444; 0.8294; 0.9876; -0.1477; -0.0528; 0.0938; 0.8257; -0.5562; 0.3634; 3.1994; -2.1551; 0; 45/60*2*pi; -0.0119; -0.0535];
addpath('Simulation');

% tether length
P.r = 1.2;
t = [0;1e3];
P.tu = [t zeros(2,3)];
P.delta = 0;
P.ddelta = x0(20);
P.dddelta = 0;
P.W0 = 0;

MPC.Ts = 0.1;

SCALE_UR = 1.25e6;
SCALE_UP = 2e5;

ur_sequence = [0:500:15000]'/SCALE_UR;

%Put to 1 if you have to recompute!
if 0
dx = @(t,x) Model_integ_ACADO(t,x,'',P);
X_ref = zeros(numel(ur_sequence),numel(x0));
for i=1:numel(ur_sequence)
    x0(21) = ur_sequence(i);
    x0(22) = 0;
    [T,Y] = ode45(dx,[0:1/500:50] ,x0);
    X_ref(i,:) = Y(end,:);
    i
end
dlmwrite('Xref_ailerons.dat',X_ref,'delimiter',' ');
end


X_ref = dlmread('Xref_ailerons.dat');

sol_S_matrix = [];

for i=1:size(X_ref,1)
    [MPC.A,MPC.B,~,~] = jacobianI(X_ref(i,:),P);  % Linearize the system at the equilibrium
    % Need to eliminate the part of the matrices relative to the dynamics of the rotation of the carousel
    MPC.A = MPC.A([1:18,21:22],[1:18,21:22]);
    MPC.B = MPC.B([1:18,21:22],2:3);

    % Define the weights for the LQR (they will be used also by the NMPC)
    We = 100;
    Ww = 1;
    % Control weights
    MPC.R = 1e-2*diag([1e-4, 100, 100]);
    % State weights
    MPC.Q = 1e-4*diag([100,  100,  100, 1, 1, 1,    ...
                    We,  We,  We,  We,  We,  We,  We,  We,  We, ...
                    Ww,  Ww,  Ww, 1e-4, 1e-4, 1, 1]);

    % Weights without the carousel rotational dynamics
    MPC.Q1 = MPC.Q([1:18,21:22],[1:18,21:22]);
    MPC.R1 = MPC.R(2:3,2:3);

    % Compute the LQR
    [K,S,e] = lqr(MPC.A,MPC.B,MPC.Q1,MPC.R1);
    [A,B] = c2d(MPC.A,MPC.B,MPC.Ts);
    [K,S,e] = dlqr(A,B,MPC.Q1,MPC.R1);
    % Add to the terminal cost matrix the missing lines / columns
    S = [ S(1:18,1:18), zeros(18,2),  S(1:18,19:20);
               zeros(2,18), 1e-4*eye(2),     zeros(2,2);
             S(19:20,1:18),  zeros(2,2), S(19:20,19:20);];
    sol_S_matrix = [sol_S_matrix; reshape(S',1,size(S,1)*size(S,2))];
end

sol_X = [X_ref zeros(size(X_ref,1),3)];

for i=1:30
    sol_X = [sol_X(1,:); sol_X; sol_X(end,:)];
    sol_S_matrix = [sol_S_matrix(1,:); sol_S_matrix; sol_S_matrix(end,:)];
end

dlmwrite('refs.dat',sol_X,'delimiter','\t');
dlmwrite('weights.dat',sol_S_matrix,'delimiter','\t');