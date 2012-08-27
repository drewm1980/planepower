close all

%!scp kurt@192.168.1.111:/home/kurt/planepower/usecases/controlExperiments/mhe_mpc.dat mhe_mpc.dat
%!cat mhe_mpc.dat | grep -v nan >mhe_mpcClean.dat
%!sed -i "1,3d" mhe_mpcClean.dat

plotDouble = 0;

data = dlmread('mhe_mpc_60rpm_stp_good.dat');
dt = 0.1;

%give the data a name
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
for i_predict=1:15:numel(t)
    [T1,Y1] = ode45(dx,[t(i_predict) t(i_predict)+T_predict] ,X3(i_predict,:)');
    T = [T; T1];
    Y = [Y; Y1];
end
fprintf('finished\n')

figure
subplot(311)
hold on
stairs(t,X3(:,1),'b')
stairs(T,Y(:,1),'r.')
ylabel('x [m]')
subplot(312)
hold on
stairs(t,X3(:,2),'b')
stairs(T,Y(:,2),'r.')
ylabel('y [m]')
subplot(313)
hold on
stairs(t,X3(:,3),'b')
stairs(T,Y(:,3),'r.')
ylabel('z [m]')

X3_markers = zeros(size(X3,1),12);

for i=1:size(X3,1)
    uvM = Model_integ_ACADO(0,X3(i,:)','markers',P);
    X3_markers(i,:) = uvM(1:12);
end
Y_markers = zeros(size(Y,1),12);
for i=1:size(Y,1)
    uvM = Model_integ_ACADO(0,Y(i,:)','markers',P);
    Y_markers(i,:) = uvM(1:12);
end

figure
subplot(611)
hold on
title('Cam 1 markers: red: predicted, blue: estimated by MHE, green: actual marker positions')
stairs(t,X3_markers(:,1),'b')
stairs(T,Y_markers(:,1),'r.')
stairs(t-0.2,Markers(:,1),'g')
subplot(612)
hold on
stairs(t,X3_markers(:,2),'b')
stairs(T,Y_markers(:,2),'r.')
stairs(t-0.2,Markers(:,2),'g')
subplot(613)
hold on
stairs(t,X3_markers(:,3),'b')
stairs(T,Y_markers(:,3),'r.')
stairs(t-0.2,Markers(:,3),'g')
subplot(614)
hold on
stairs(t,X3_markers(:,4),'b')
stairs(T,Y_markers(:,4),'r.')
stairs(t-0.2,Markers(:,4),'g')
subplot(615)
hold on
stairs(t,X3_markers(:,5),'b')
stairs(T,Y_markers(:,5),'r.')
stairs(t-0.2,Markers(:,5),'g')
subplot(616)
hold on
stairs(t,X3_markers(:,6),'b')
stairs(T,Y_markers(:,6),'r.')
stairs(t-0.2,Markers(:,6),'g')

figure
subplot(611)
hold on
title('Cam 2 markers: red: predicted, blue: estimated by MHE, green: actual marker positions')
stairs(t,X3_markers(:,7),'b')
stairs(T,Y_markers(:,7),'r.')
stairs(t-0.2,Markers(:,7),'g')
subplot(612)
hold on
stairs(t,X3_markers(:,8),'b')
stairs(T,Y_markers(:,8),'r.')
stairs(t-0.2,Markers(:,8),'g')
subplot(613)
hold on
stairs(t,X3_markers(:,9),'b')
stairs(T,Y_markers(:,9),'r.')
stairs(t-0.2,Markers(:,9),'g')
subplot(614)
hold on
stairs(t,X3_markers(:,10),'b')
stairs(T,Y_markers(:,10),'r.')
stairs(t-0.2,Markers(:,10),'g')
subplot(615)
hold on
stairs(t,X3_markers(:,11),'b')
stairs(T,Y_markers(:,11),'r.')
stairs(t-0.2,Markers(:,11),'g')
subplot(616)
hold on
stairs(t,X3_markers(:,12),'b')
stairs(T,Y_markers(:,12),'r.')
stairs(t-0.2,Markers(:,12),'g')

% figure
% subplot(121)
% hold on
% plot(Markers(:,1),Markers(:,2),'r.')
% plot(Markers(:,3),Markers(:,4),'g.')
% plot(Markers(:,5),Markers(:,6),'b.')
% title('Measured marker positions of Camera 1')
% subplot(122)
% hold on
% plot(X3_markers(:,1),X3_markers(:,2),'r.')
% plot(X3_markers(:,3),X3_markers(:,4),'g.')
% plot(X3_markers(:,5),X3_markers(:,6),'b.')
% title('Estimated marker positions of Camera 1')
% 
% figure
% subplot(121)
% hold on
% plot(Markers(:,7),Markers(:,8),'r.')
% plot(Markers(:,9),Markers(:,10),'g.')
% plot(Markers(:,11),Markers(:,12),'b.')
% title('Measured marker positions of Camera 2')
% subplot(122)
% hold on
% plot(X3_markers(:,7),X3_markers(:,8),'r.')
% plot(X3_markers(:,9),X3_markers(:,10),'g.')
% plot(X3_markers(:,11),X3_markers(:,12),'b.')
% title('Estimated marker positions of Camera 2')
