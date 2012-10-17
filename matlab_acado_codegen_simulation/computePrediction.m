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
    [T1,Y1] = ode45(dx,[t(i_predict) t(i_predict)+T_predict] ,X1(i_predict,:)');
    T = [T; T1];
    Y = [Y; Y1];
end
fprintf('finished\n')


X1_markers = zeros(size(X1,1),12);

for i=1:size(X1,1)
    uvM = Model_integ_ACADO(0,X1(i,:)','markers',P);
    X1_markers(i,:) = uvM(1:12);
end
Y_markers = zeros(size(Y,1),12);
for i=1:size(Y,1)
    uvM = Model_integ_ACADO(0,Y(i,:)','markers',P);
    Y_markers(i,:) = uvM(1:12);
end

X1_imu = zeros(size(X1,1),6);
for i=1:size(X1,1)
    X1_imu(i,:) = Model_integ_ACADO(t(i),X1(i,:)','IMU',P);
end
Y_imu = zeros(size(Y,1),6);
for i=1:size(Y,1)
    Y_imu(i,:) = Model_integ_ACADO(0,Y(i,:)','IMU',P);
end

t_iterates = zeros(3*size(X1,1),1);
X_iterates = zeros(3*size(X1,1),size(X1,2));

for i=1:numel(t)
    t_iterates(3*i-2) = t(i);
    t_iterates(3*i-1) = t(i)+4e-3;
    t_iterates(3*i) = t(i)+8e-3;
    X_iterates(3*i-2,:) = X1(i,:);
    X_iterates(3*i-1,:) = X2(i,:);
    X_iterates(3*i,:) = X3(i,:);
end

X_imu_iterates = zeros(size(X_iterates,1),6);
for i=1:size(X_iterates,1)
    X_imu_iterates(i,:) = Model_integ_ACADO(t_iterates(i),X_iterates(i,:)','IMU',P);
end
