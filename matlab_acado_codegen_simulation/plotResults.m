close all

%!scp kurt@192.168.1.111:/home/kurt/planepower/usecases/controlExperiments/mhe_mpc.dat mhe_mpc.dat
%!cat mhe_mpc.dat | grep -v nan >mhe_mpcClean.dat
%!sed -i "1,3d" mhe_mpcClean.dat

plotDouble = 0;

data = dlmread('mhe_mpc_60rpm_stp_good.dat');

data = data(1:end-2,:);

for i =1:1 %just so that I can make this block small.
    NX = 22;
    NU = 3;
    NY_MARKERS = 12;
    NY_IMU = 6;
    NY_POSE = 12;
    NY_ENC = 2;
    t = data(:,1);
    X1 = data(:,2:2+NX-1); %estimate after first sqp-iteration
    X2 = data(:,2+NX:2+NX-1+NX); %estimate after second sqp-iteration
    X3 = data(:,2+2*NX:2+NX-1+2*NX); %estimate after third sqp-iteration
    XPred = data(:,2+3*NX:2+NX-1+3*NX); %prediction of MHE, used for initialisation
    XRef = data(:,2+4*NX:2+NX-1+4*NX); % Reference for controller
    U = data(:,2+5*NX:2+5*NX+NU-1); % aileron and elevator deflection (ur1,ur2,up)
    dU = data(:,2+5*NX+NU:2+5*NX+NU-1+NU); % derivative of controls (dur1, dur2, dup)
    Markers = data(:,2+5*NX+2*NU:2+5*NX+2*NU+NY_MARKERS-1); % position of the markers
    IMU = data(:,2+5*NX+2*NU+NY_MARKERS:2+5*NX+2*NU+NY_MARKERS+NY_IMU-1); % Imu data (omega, acc)
    POSE = data(:,2+5*NX+2*NU+NY_MARKERS+NY_IMU:2+5*NX+2*NU+NY_MARKERS+NY_IMU+NY_POSE-1); % Pose estimate from Andrew's code
    ENC = data(:,2+5*NX+2*NU+NY_MARKERS+NY_IMU+NY_POSE:2+5*NX+2*NU+NY_MARKERS+NY_IMU+NY_POSE+NY_ENC-1); % Encoder data (delta, ddelta)

    x_1 = X1(:,1); y_1 = X1(:,2); z_1 = X1(:,3); 
    dx_1 = X1(:,4); dy_1 = X1(:,5); dz_1 = X1(:,6); 
    e11_1 = X1(:,7); e12_1 = X1(:,8); e13_1 = X1(:,9); 
    e21_1 = X1(:,10); e22_1 = X1(:,11); e23_1 = X1(:,12); 
    e31_1 = X1(:,13); e32_1 = X1(:,14); e33_1 = X1(:,15); 
    w1_1 = X1(:,16); w2_1 = X1(:,17); w3_1 = X1(:,18); 
    delta_1 = X1(:,19); ddelta_1 = X1(:,20); 
    ur_1 = X1(:,21); up_1 = X1(:,22);

    x_2 = X2(:,1); y_2 = X2(:,2); z_2 = X2(:,3); 
    dx_2 = X2(:,4); dy_2 = X2(:,5); dz_2 = X2(:,6); 
    e11_2 = X2(:,7); e12_2 = X2(:,8); e13_2 = X2(:,9); 
    e21_2 = X2(:,10); e22_2 = X2(:,11); e23_2 = X2(:,12); 
    e31_2 = X2(:,13); e32_2 = X2(:,14); e33_2 = X2(:,15); 
    w1_2 = X2(:,16); w2_2 = X2(:,17); w3_2 = X2(:,18); 
    delta_2 = X2(:,19); ddelta_2 = X2(:,20); 
    ur_2 = X2(:,21); up_2 = X2(:,22);

    x_3 = X3(:,1); y_3 = X3(:,2); z_3 = X3(:,3); 
    dx_3 = X3(:,4); dy_3 = X3(:,5); dz_3 = X3(:,6); 
    e11_3 = X3(:,7); e12_3 = X3(:,8); e13_3 = X3(:,9); 
    e21_3 = X3(:,10); e22_3 = X3(:,11); e23_3 = X3(:,12); 
    e31_3 = X3(:,13); e32_3 = X3(:,14); e33_3 = X3(:,15); 
    w1_3 = X3(:,16); w2_3 = X3(:,17); w3_3 = X3(:,18); 
    delta_3 = X3(:,19); ddelta_3 = X3(:,20); 
    ur_3 = X3(:,21); up_3 = X3(:,22);

    x_Pred = XPred(:,1); y_Pred = XPred(:,2); z_Pred = XPred(:,3); 
    dx_Pred = XPred(:,4); dy_Pred = XPred(:,5); dz_Pred = XPred(:,6); 
    e11_Pred = XPred(:,7); e12_Pred = XPred(:,8); e13_Pred = XPred(:,9); 
    e21_Pred = XPred(:,10); e22_Pred = XPred(:,11); e23_Pred = XPred(:,12); 
    e31_Pred = XPred(:,13); e32_Pred = XPred(:,14); e33_Pred = XPred(:,15); 
    w1_Pred = XPred(:,16); w2_Pred = XPred(:,17); w3_Pred = XPred(:,18); 
    delta_Pred = XPred(:,19); ddelta_Pred = XPred(:,20); 
    ur_Pred = XPred(:,21); up_Pred = XPred(:,22);

    x_Ref = XRef(:,1); y_Ref = XRef(:,2); z_Ref = XRef(:,3); 
    dx_Ref = XRef(:,4); dy_Ref = XRef(:,5); dz_Ref = XRef(:,6); 
    e11_Ref = XRef(:,7); e12_Ref = XRef(:,8); e13_Ref = XRef(:,9); 
    e21_Ref = XRef(:,10); e22_Ref = XRef(:,11); e23_Ref = XRef(:,12); 
    e31_Ref = XRef(:,13); e32_Ref = XRef(:,14); e33_Ref = XRef(:,15); 
    w1_Ref = XRef(:,16); w2_Ref = XRef(:,17); w3_Ref = XRef(:,18); 
    delta_Ref = XRef(:,19); ddelta_Ref = XRef(:,20); 
    ur_Ref = XRef(:,21); up_Ref = XRef(:,22);
    
    ur = U(:,1);
    up = U(:,3);
    
    dur = dU(:,1);
    dup = dU(:,3);
    
    x_POSE = POSE(:,1); y_POSE = POSE(:,2); z_POSE = POSE(:,3);
    e11_POSE = POSE(:,4); e12_POSE = POSE(:,5); e13_POSE = POSE(:,6); 
    e21_POSE = POSE(:,7); e22_POSE = POSE(:,8); e23_POSE = POSE(:,9); 
    e31_POSE = POSE(:,10); e32_POSE = POSE(:,11); e33_POSE = POSE(:,12); 
    
    %delta_ENC = ENC(:,1); ddelta_ENC = ENC(:,2);
end

plotting = 1;

roll_3 = zeros(size(e12_3));
roll_Ref = zeros(size(e12_3));
for i=1:numel(roll_3)
    roll_3(i) = atan2(e12_3(i),e32_3(i));
    roll_Ref(i) = atan2(e12_Ref(i),e32_Ref(i));
end

if plotting

figure
subplot(311)
title('Position')
hold on
ylabel('x [m]')
stairs(t,x_1,'b')
stairs(t,x_Ref,'k')
subplot(312)
hold on
ylabel('y [m]')
stairs(t,y_1,'b')
stairs(t,y_Ref,'k')
subplot(313)
hold on
ylabel('z [m]')
stairs(t,z_1,'b')
stairs(t,z_Ref,'k')
xlabel('Time [s]')

figure
subplot(311)
hold on
ylabel('dx [m/s]')
title('Velocity')
stairs(t,dx_1,'b')
stairs(t,dx_Ref,'k')
subplot(312)
hold on
ylabel('dy [m/s]')
stairs(t,dy_1,'b')
stairs(t,dy_Ref,'k')
subplot(313)
hold on
ylabel('dz [m/s]')
stairs(t,dz_1,'b')
stairs(t,dz_Ref,'k')
xlabel('Time [s]')


figure
subplot(311)
hold on
ylabel('omega_x [degree/s]')
title('Angular velocities')
stairs(t,w1_1*180/pi,'b')
stairs(t,w1_Ref*180/pi,'k')
axis tight
subplot(312)
hold on
ylabel('omega_y [degree/s]')
stairs(t,w2_1*180/pi,'b')
stairs(t,w2_Ref*180/pi,'k')
axis tight
subplot(313)
hold on
ylabel('omega_z [degree/s]')
stairs(t,w3_1*180/pi,'b')
stairs(t,w3_Ref*180/pi,'k')
axis tight
xlabel('Time [s]')

% figure
% subplot(211)
% hold on
% ylabel('ur (red) vs w1 (blue), in degree, but divided by 100')
% stairs(t,ur/32767,'r')
% stairs(t,w1_1*180/pi/100,'b')
% %plot(t,32767,'k')
% %plot(t,-32767,'k')
% axis tight
% subplot(212)
% hold on
% ylabel('up (red) vs w2 (blue), in degree, but divided by 10000')
% stairs(t,up/20000,'r')
% stairs(t,w2_1*180/pi/10000,'b')
% %plot(t,20000,'k')
% %plot(t,-20000,'k')
% axis tight
% xlabel('Time [s]')

figure
subplot(331)
hold on
ylabel('e11')
stairs(t,e11_1,'b')
stairs(t,e11_Ref,'k')
subplot(334)
hold on
ylabel('e21')
stairs(t,e21_1,'b')
stairs(t,e21_Ref,'k')
subplot(337)
hold on
ylabel('e31')
stairs(t,e31_1,'b')
stairs(t,e31_Ref,'k')
xlabel('Time [s]')
subplot(332)
hold on
ylabel('e12')
title('Rotation matrix')
stairs(t,e12_1,'b')
stairs(t,e12_Ref,'k')
subplot(335)
hold on
ylabel('e22')
stairs(t,e22_1,'b')
stairs(t,e22_Ref,'k')
subplot(338)
hold on
ylabel('e32')
stairs(t,e32_1,'b')
stairs(t,e32_Ref,'k')
xlabel('Time [s]')
subplot(333)
hold on
ylabel('e13')
stairs(t,e13_1,'b')
stairs(t,e13_Ref,'k')
subplot(336)
hold on
ylabel('e23')
stairs(t,e23_1,'b')
stairs(t,e23_Ref,'k')
subplot(339)
hold on
ylabel('e33')
stairs(t,e33_1,'b')
stairs(t,e33_Ref,'k')
xlabel('Time [s]')

figure
hold on
stairs(t,roll_3/pi*180,'b');
stairs(t,roll_Ref/pi*180,'k');
xlabel('Time [s]')
ylabel('Roll angle [deg]')
title('Roll')

figure
subplot(211)
stairs(t,ur)
ylabel('ur')
title('Control inputs')
subplot(212)
stairs(t,up)
ylabel('up')
xlabel('Time [s]')

end
