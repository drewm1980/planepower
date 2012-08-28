addpath('Simulation');

% tether length
P.r = 1.2;
% dddelta, dur, dup
P.tu = [t zeros(size(dur)) dur dup];

dx = @(t,x) Model_integ_ACADO(t,x,'',P);

T_predict = 1; % time to integrate forward

N_predict = T_predict/dt; % Number of steps to integrate

i_predict = 60; %node to integrate from
fprintf('integrating...\n')
T = [];
Y = [];
for i_predict=5:11:numel(t)
    [T1,Y1] = ode45(dx,[t(i_predict) t(i_predict)+T_predict] ,X3(i_predict,:)');
    T = [T; T1];
    Y = [Y; Y1];
end
fprintf('finished\n')