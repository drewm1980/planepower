close all

!scp kurt@192.168.1.111:/home/kurt/planepower/usecases/controlExperiments/closed_loop_lqr_data.dat stateAndControl.dat
!cat stateAndControl.dat | grep -v nan >stateAndControlClean.dat
!sed -i "1,3d" stateAndControlClean.dat

data = dlmread('stateAndControlClean.dat'); 


t = data(:,1);
dt = 0.1;
t = [0:dt:dt*(size(data,1)-1)]';

x_est = data(:,2); y_est = data(:,3); z_est = data(:,4);
dx_est = data(:,5); dy_est = data(:,6); dz_est = data(:,7);
e11_est = data(:,8); e12_est = data(:,9); e13_est = data(:,10); 
e21_est = data(:,11); e22_est = data(:,12); e23_est = data(:,13);
e31_est = data(:,14); e32_est = data(:,15); e33_est = data(:,16);
w1_est = data(:,17); w2_est = data(:,18); w3_est = data(:,19);
delta_est = data(:,20); ddelta_est = data(:,21);
ur_est = data(:,22); up_est = data(:,23);
ur_applied = data(:,24); up_applied = data(:,26);
dur_applied = data(:,27); dup_applied = data(:,29);
markers = data(:,30:41);
imu = data(:,42:47);
omega = imu(:,1:3);
acc = imu(:,4:6);

Xt = data(:,2:23);
Xt(:,21) = ur_applied;
Xt(:,22) = up_applied;

addpath('Simulation');

P.r = 1.2;
P.tu = [t zeros(size(dur_applied)) dur_applied dup_applied];

dx = @(t,x) Model_integ_ACADO(t,x,'',P);

T_predict = 1; % time to integrate forward

N_predict = T_predict/dt; % Number of steps to integrate

i_predict = 150; %node to integrate from
fprintf('integrating...\n')
[T,Y] = ode45(dx,[t(i_predict) t(i_predict)+T_predict] ,Xt(i_predict,:)');
fprintf('finished\n')

figure
subplot(311)
hold on
stairs(t,Xt(:,1),'b')
stairs(T,Y(:,1),'r')
ylabel('x [m]')
subplot(312)
hold on
stairs(t,Xt(:,2),'b')
stairs(T,Y(:,2),'r')
ylabel('y [m]')
subplot(313)
hold on
stairs(t,Xt(:,3),'b')
stairs(T,Y(:,3),'r')
ylabel('z [m]')

Xt_markers = zeros(size(Xt,1),12);

for i=1:size(Xt,1)
    uvM = Model_integ_ACADO(0,Xt(i,:)','markers',P);
    Xt_markers(i,:) = uvM(1:12);
end
Y_markers = zeros(size(Y,1),12);
for i=1:size(Y,1)
    uvM = Model_integ_ACADO(0,Y(i,:)','markers',P);
    Y_markers(i,:) = uvM(1:12);
end

figure
subplot(611)
hold on
stairs(t,Xt_markers(:,1),'b')
stairs(T,Y_markers(:,1),'r')
stairs(t,markers(:,1),'g')
subplot(612)
hold on
stairs(t,Xt_markers(:,2),'b')
stairs(T,Y_markers(:,2),'r')
stairs(t,markers(:,2),'g')
subplot(613)
hold on
stairs(t,Xt_markers(:,3),'b')
stairs(T,Y_markers(:,3),'r')
stairs(t,markers(:,3),'g')
subplot(614)
hold on
stairs(t,Xt_markers(:,4),'b')
stairs(T,Y_markers(:,4),'r')
stairs(t,markers(:,4),'g')
subplot(615)
hold on
stairs(t,Xt_markers(:,5),'b')
stairs(T,Y_markers(:,5),'r')
stairs(t,markers(:,5),'g')
subplot(616)
hold on
stairs(t,Xt_markers(:,6),'b')
stairs(T,Y_markers(:,6),'r')
stairs(t,markers(:,6),'g')

figure
subplot(611)
hold on
stairs(t,Xt_markers(:,7),'b')
stairs(T,Y_markers(:,7),'r')
stairs(t,markers(:,7),'g')
subplot(612)
hold on
stairs(t,Xt_markers(:,8),'b')
stairs(T,Y_markers(:,8),'r')
stairs(t,markers(:,8),'g')
subplot(613)
hold on
stairs(t,Xt_markers(:,9),'b')
stairs(T,Y_markers(:,9),'r')
stairs(t,markers(:,9),'g')
subplot(614)
hold on
stairs(t,Xt_markers(:,10),'b')
stairs(T,Y_markers(:,10),'r')
stairs(t,markers(:,10),'g')
subplot(615)
hold on
stairs(t,Xt_markers(:,11),'b')
stairs(T,Y_markers(:,11),'r')
stairs(t,markers(:,11),'g')
subplot(616)
hold on
stairs(t,Xt_markers(:,12),'b')
stairs(T,Y_markers(:,12),'r')
stairs(t,markers(:,12),'g')
