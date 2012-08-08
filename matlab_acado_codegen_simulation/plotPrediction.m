%close all

%!scp kurt@192.168.1.111:/home/kurt/planepower/usecases/controlExperiments/closed_loop_lqr_data.dat stateAndControl.dat
%!cat stateAndControl.dat | grep -v nan >stateAndControlClean.dat
%!sed -i "1,3d" stateAndControlClean.dat

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

Xt = data(:,2:23);
Xt(:,21) = ur_applied;
Xt(:,22) = up_applied;

addpath('Simulation');

P.r = 1.2;
P.tu = [t zeros(size(dur_applied)) dur_applied dup_applied];

dx = @(t,x) Model_integ_ACADO(t,x,'',P);

N_predict = 10;

Xt_predict = zeros(size(Xt));

for i=1:size(Xt_predict,1)-N_predict-1
%for i=1:73
    fprintf(strcat(['running simulation ',int2str(i),' of ',int2str(size(Xt_predict,1)),'\n']) )
    [T,Y] = ode45(dx,[(i-1)*dt (i-1)*dt+N_predict*dt] ,Xt(i,:)');
    Xt_predict(i,:) = Y(end,:);
end

figure
plot(t(1:73),Xt(1:73,1),'b')
hold on
plot(t(1:73),Xt_predict(1:73,1),'r')