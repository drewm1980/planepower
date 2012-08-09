close all

!scp kurt@192.168.1.111:/home/kurt/planepower/usecases/controlExperiments/closed_loop_lqr_data.dat closed_loop_lqr_data.dat
!cat closed_loop_lqr_data.dat | grep -v nan >closed_loop_lqr_dataClean.dat
!sed -i "1,3d" closed_loop_lqr_dataClean.dat

plotDouble = 0;

data = dlmread('closed_loop_lqr_dataClean.dat'); % blue plotted
data1 = dlmread('closed_loop_lqr_dataClean1.dat'); % Red plotted
data = data(10:end,:);
data1 = data1(10:end,:);
if(size(data,2) == 26)
    data = [data zeros(size(data,1),25)];
end


t = data(:,1);
x_est = data(:,2);
y_est = data(:,3);
z_est = data(:,4);
dx_est = data(:,5);
dy_est = data(:,6);
dz_est = data(:,7);
e11_est = data(:,8);
e12_est = data(:,9);
e13_est = data(:,10);
e21_est = data(:,11);
e22_est = data(:,12);
e23_est = data(:,13);
e31_est = data(:,14);
e32_est = data(:,15);
e33_est = data(:,16);
w1_est = data(:,17);
w2_est = data(:,18);
w3_est = data(:,19);
delta_est = data(:,20);
ddelta_est = data(:,21);
ur_est = data(:,22);
up_est = data(:,23);
x_ref = data(:,24);
y_ref = data(:,25);
z_ref = data(:,26);
dx_ref = data(:,27);
dy_ref = data(:,28);
dz_ref = data(:,29);
e11_ref = data(:,30);
e12_ref = data(:,31);
e13_ref = data(:,32);
e21_ref = data(:,33);
e22_ref = data(:,34);
e23_ref = data(:,35);
e31_ref = data(:,36);
e32_ref = data(:,37);
e33_ref = data(:,38);
w1_ref = data(:,39);
w2_ref = data(:,40);
w3_ref = data(:,41);
delta_ref = data(:,42);
ddelta_ref = data(:,43);
ur_ref = data(:,44);
up_ref = data(:,45);
ur = data(:,46);
ur = data(:,47);
up = data(:,48);
dur = data(:,49);
dur = data(:,50);
dup = data(:,51);


t1 = data1(:,1);
x_est1 = data1(:,2);
y_est1 = data1(:,3);
z_est1 = data1(:,4);
dx_est1 = data1(:,5);
dy_est1 = data1(:,6);
dz_est1 = data1(:,7);
e11_est1 = data1(:,8);
e12_est1 = data1(:,9);
e13_est1 = data1(:,10);
e21_est1 = data1(:,11);
e22_est1 = data1(:,12);
e23_est1 = data1(:,13);
e31_est1 = data1(:,14);
e32_est1 = data1(:,15);
e33_est1 = data1(:,16);
w1_est1 = data1(:,17);
w2_est1 = data1(:,18);
w3_est1 = data1(:,19);
delta_est1 = data1(:,20);
ddelta_est1 = data1(:,21);
ur_est1 = data1(:,22);
up_est1 = data1(:,23);
x_ref1 = data1(:,24);
y_ref1 = data1(:,25);
z_ref1 = data1(:,26);
dx_ref1 = data1(:,27);
dy_ref1 = data1(:,28);
dz_ref1 = data1(:,29);
e11_ref1 = data1(:,30);
e12_ref1 = data1(:,31);
e13_ref1 = data1(:,32);
e21_ref1 = data1(:,33);
e22_ref1 = data1(:,34);
e23_ref1 = data1(:,35);
e31_ref1 = data1(:,36);
e32_ref1 = data1(:,37);
e33_ref1 = data1(:,38);
w1_ref1 = data1(:,39);
w2_ref1 = data1(:,40);
w3_ref1 = data1(:,41);
delta_ref1 = data1(:,42);
ddelta_ref1 = data1(:,43);
ur_ref1 = data1(:,44);
up_ref1 = data1(:,45);
ur1 = data1(:,46);
ur1 = data1(:,47);
up1 = data1(:,48);
dur1 = data1(:,49);
dur1 = data1(:,50);
dup1 = data1(:,51);

figure
subplot(311)
stairs(t,x_est,'b')
ylabel('x [m]')
hold on
if plotDouble
    stairs(t1,x_est1,'r')
end
stairs(t,x_ref,'k')
title('position (x,y,z)')
subplot(312)
stairs(t,y_est,'b')
ylabel('y [m]')
hold on
if plotDouble
    stairs(t1,y_est1,'r')
end
stairs(t,y_ref,'k')
subplot(313)
stairs(t,z_est,'b')
ylabel('z [m]')
hold on
if plotDouble
    stairs(t1,y_est1,'r')
end
stairs(t,z_ref,'k')

figure
subplot(311)
stairs(t,dx_est,'r')
ylabel('dx [m/s]')
hold on
stairs(t,dx_ref,'k')
title('velocity (dx,dy,dz)')
subplot(312)
stairs(t,dy_est,'r')
ylabel('dy [m/s]')
hold on
stairs(t,dy_ref,'k')
subplot(313)
stairs(t,dz_est,'r')
ylabel('dz [m/s]')
hold on
stairs(t,dz_ref,'k')

figure
subplot(311)
stairs(t,w1_est*180/pi,'b')
hold on
if plotDouble
    stairs(t1,w1_est1*180/pi,'r')
end
stairs(t,w1_ref*180/pi,'k')
ylabel('omega_x [degree/s]')
axis tight
subplot(312)
stairs(t,w2_est*180/pi,'b')
hold on
if plotDouble
    stairs(t1,w2_est1*180/pi,'r')
end
stairs(t,w2_ref*180/pi,'k')
ylabel('omega_y [degree/s]')
axis tight
subplot(313)
stairs(t,w3_est*180/pi,'b')
hold on
if plotDouble
    stairs(t1,w3_est1*180/pi,'r')
end
stairs(t,w3_ref*180/pi,'k')
ylabel('omega_z [degree/s]')
axis tight

figure
subplot(211)
stairs(t,ur/32767,'r')
hold on
stairs(t,w1_est*180/pi/100,'b')
ylabel('ur (red) vs w1 (blue), in degree, but divided by 100')
hold on
%plot(t,32767,'k')
%plot(t,-32767,'k')
axis tight
subplot(212)
stairs(t,up/20000,'r')
hold on
stairs(t,w2_est*180/pi/10000,'b')
ylabel('up (red) vs w2 (blue), in degree, but divided by 10000')
hold on
%plot(t,20000,'k')
%plot(t,-20000,'k')
axis tight

figure
subplot(311)
stairs(t,e11_est,'b')
hold on
stairs(t,e11_ref,'k')
ylabel('e11')
subplot(312)
stairs(t,e21_est,'b')
hold on
stairs(t,e21_ref,'k')
ylabel('e21')
subplot(313)
stairs(t,e31_est,'b')
hold on
stairs(t,e31_ref,'k')
ylabel('e31')

figure
subplot(311)
stairs(t,e12_est,'b')
hold on
stairs(t,e12_ref,'k')
ylabel('e12')
subplot(312)
stairs(t,e22_est,'b')
hold on
stairs(t,e22_ref,'k')
ylabel('e22')
subplot(313)
stairs(t,e32_est,'b')
hold on
stairs(t,e32_ref,'k')
ylabel('e32')

figure
subplot(311)
stairs(t,e13_est,'b')
hold on
stairs(t,e13_ref,'k')
ylabel('e13')
subplot(312)
stairs(t,e23_est,'b')
hold on
stairs(t,e23_ref,'k')
ylabel('e23')
subplot(313)
stairs(t,e33_est,'b')
hold on
stairs(t,e33_ref,'k')
ylabel('e33')