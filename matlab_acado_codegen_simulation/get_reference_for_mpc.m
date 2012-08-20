clear all
close all
clc

addpath('Simulation','Matlabfunctions','code_export_nmpc','code_export_mhe')

z_start = -0.1189362777884522; % Starting height of the ramp
z_end = -0.04; % End height of the ramp
z_start = -0.4; % Starting height of the ramp
z_end = -0.34; % End height of the ramp

Ts = 0.1; % Sampling time
Tc = 10; % Time of ramp
z = [];
if true %Generate a sine-wave reverence
    z_min = -0.1189362777884522;%minimum of the sine
    z_max = -0.04;%max of the sine
    Tp = 0.9; %Period of the sine
    N = 10; % Number of periods
    A = (z_max-z_min)/2;%amplitude of the sine
    M = (z_max+z_min)/2;%level of the sine
    t = 0:Ts:N*Tp;
    omega = 2*pi/Tp;
    t = t-Tp/4;% shift time so that we start in a minimum of the sine (on the steady state)
    z = M+A*sin(omega*t);%z-profile
    %Add constant reference at beginning and end:
    T_steady = 5;%add 5 seconds of constant reference
    t_append = 0:Ts:T_steady-Ts;
    z_append = z_min*ones(1,T_steady/Ts);
    z = [z_append z z_append];
    t = 0:Ts:Ts*(numel(z)-1);
end

delta_z = (z_end-z_start)/(Tc/Ts);
MPC.is_init = 0;

for i=0:(Tc/Ts)


% MPC settings
MPC.Tc = 1; % horizon in seconds
MPC.Ncvp = 10; % number of cvp
MPC.Ts  = MPC.Tc/MPC.Ncvp; % sampling time
MPC.Nref = MPC.Ncvp+1; % 500;% % number of elements in the reference file
% Multiplying factor for the terminal cost
MPC.Sfac = 1; % factor multiplying the terminal cost

% MHE settings
MHE.Tc = 1;
MHE.Ncvp = 10;
MHE.Ts  = MHE.Tc/MHE.Ncvp;
MHE.Nref = MHE.Ncvp+1;%500;%

% Consistency check for the sampling times in MPC and MHE
if MPC.Ts ~= MHE.Ts
    error('Sampling times in MPC and MHE do not match')
end

% Set integration parameters
Sim.intoptions.AbsTol = 1e-8;
Sim.intoptions.RelTol = 1e-8;
Sim.intoptions.MaxStep = 1/10;
Sim.intoptions.Events = 'off';

% Simulation settings
Sim.Tf = 8;         
% simulation final time
Sim.decoupleMPC_MHE = 0;    % Set different from 0 to have MPC and MHE running in parallel, but independently, i.e.: MPC gets perfect state estimates

% Wind parameters
Sim.W0 = 0;         % Wind known by the controller
Sim.DeltaW = 0;     % Windgusts or unknown wind

% Sensor noise
Sim.Noise.is = 1;           % set to 0 to kill the noise
Sim.Noise.factor = 1/40;    % noise: factor*scale_of_related_state = max_noise

% Initial condition
MPC.Ref.z = z_start + i*delta_z ;         % reference trajectory height (relative to the arm)
MPC.Ref.r = 1.2;            % tether length
MPC.Ref.delta = 0;          % initial carousel angle
MPC.Ref.RPM = 37;           % carousel rotational velocity
MPC.Ref.ddelta = 2*pi*MPC.Ref.RPM/60.;
Sim.r = MPC.Ref.r;          % copy the tether length (just for the ease of use)

% Define the reference by stacking data in time vectors
for k = 1:(Sim.Tf/MPC.Ts + 2)
    MPC.Ref.time(k,1) = MPC.Ts*(k-1);
    MPC.Ref.vars(k,1:3)   = [MPC.Ref.z MPC.Ref.r MPC.Ref.ddelta];
    
    % apply a step change after 1 second
    if MPC.Ref.time(k,1) >= 1 && MPC.Ref.time(k,1) < 5 
        MPC.Ref.z = 0.1;
    elseif MPC.Ref.time(k,1) >= 5
        MPC.Ref.z = 0.0;
    end
end

display('------------------------------------------------------------------')
display('               Initialization')
display('------------------------------------------------------------------')

% Initialize (first MPC, then MHE)
[MPC,Sim] = initializeMPC(MPC,Sim);

sol_S(:,:,i+1) = MPC.S;
sol_K(:,:,i+1) = MPC.K;
sol_X(:,i+1) = MPC.Xref(1,:);
MPC.is_init = 1;
end
sol_K_matrix = [];
for i=1:size(sol_K,3)
    sol_K_matrix = [sol_K_matrix; sol_K(:,:,i)];
end

sol_S_matrix = [];
for i=1:size(sol_S,3)
    sol_S_matrix = [sol_S_matrix; reshape(sol_S(:,:,i)',1,size(sol_S(:,:,i),1)*size(sol_S(:,:,i),2))];
end

for i=1:30
    sol_X = [sol_X(:,1) sol_X];
    sol_S_matrix = [sol_S_matrix(1,:); sol_S_matrix];
end

for i=1:30
    sol_X = [sol_X sol_X(:,end)];
    sol_S_matrix = [sol_S_matrix; sol_S_matrix(end,:)];
end

sol_X =[sol_X;zeros(3,size(sol_X,2))];
sol_X = sol_X';

sol_X = [sol_X; flipud(sol_X)];
sol_S_matrix = [sol_S_matrix; flipud(sol_S_matrix)];
sol_X_1 = sol_X;
sol_S_matrix_1 = sol_S_matrix;
for i=1:20
    sol_X = [sol_X; sol_X_1];
    sol_S_matrix = [sol_S_matrix; sol_S_matrix_1];
end

dlmwrite('refs.dat',sol_X,'delimiter','\t');
dlmwrite('weights.dat',sol_S_matrix,'delimiter','\t');