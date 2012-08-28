close all

loadData;
computePrediction;

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

X3_imu = zeros(size(X3,1),6);
for i=1:size(X3,1)
    X3_imu(i,:) = Model_integ_ACADO(t(i),X3(i,:)','IMU',P);
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

figure
subplot(311)
hold on
title('Omega: red: predicted, blue: estimated by MHE, green: actual measurements')
plot(t,X3_imu(:,1),'b')
plot(T,Y_imu(:,1),'r.')
plot(t,IMU(:,1),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
subplot(312)
hold on
plot(t,X3_imu(:,2),'b')
plot(T,Y_imu(:,2),'r.')
plot(t,IMU(:,2),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
subplot(313)
hold on
plot(t,X3_imu(:,3),'b')
plot(T,Y_imu(:,3),'r.')
plot(t,IMU(:,3),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')

figure
%subplot(311)
hold on
title('Acceleration: red: predicted, blue: estimated by MHE, green: actual measurements')
plot(t,X3_imu(:,4),'b')
plot(T,Y_imu(:,4),'r.')
plot(t,IMU(:,4),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(312)
%hold on
plot(t,X3_imu(:,5),'b')
plot(T,Y_imu(:,5),'r.')
plot(t,IMU(:,5),'g')
%legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(313)
%hold on
plot(t,X3_imu(:,6),'b')
plot(T,Y_imu(:,6),'r.')
plot(t,IMU(:,6),'g')
%legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')


figure
%subplot(311)
hold on
title('Omega: red: predicted, blue: estimated by MHE, green: actual measurements')
plot(t_iterates,X_imu_iterates(:,1),'b.')
plot(t,IMU(:,1),'g.')
legend('MHE estimate','Actual measurement')
%subplot(312)
%hold on
plot(t_iterates,X_imu_iterates(:,2),'b.')
plot(t,IMU(:,2),'g.')
%legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(313)
%hold on
plot(t_iterates,X_imu_iterates(:,3),'b.')
plot(t,IMU(:,3),'g.')
%legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')

figure
%subplot(311)
hold on
title('Acceleration: red: predicted, blue: estimated by MHE, green: actual measurements')
plot(t_iterates,X_imu_iterates(:,4),'b.')
plot(t,IMU(:,4),'g.')
legend('MHE estimate','Actual measurement')
%subplot(312)
%hold on
plot(t_iterates,X_imu_iterates(:,5),'b.')
plot(t,IMU(:,5),'g.')
%legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(313)
%hold on
plot(t_iterates,X_imu_iterates(:,6),'b.')
plot(t,IMU(:,6),'g.')
%legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')

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
