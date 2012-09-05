close all

loadData;
computePrediction;

figure
subplot(311)
hold on
stairs(t,X3(:,1),'b')
stairs(T,Y(:,1),'r.','MarkerSize',2)
ylabel('x [m]')
subplot(312)
hold on
stairs(t,X3(:,2),'b')
stairs(T,Y(:,2),'r.','MarkerSize',2)
ylabel('y [m]')
subplot(313)
hold on
stairs(t,X3(:,3),'b')
stairs(T,Y(:,3),'r.','MarkerSize',2)
ylabel('z [m]')
xlabel('Time [s]')

figure
subplot(611)
hold on
%title('Camera 1 marker positions [pixels]')
stairs(t,X3_markers(:,1),'b')
stairs(T,Y_markers(:,1),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,1),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
subplot(612)
hold on
stairs(t,X3_markers(:,2),'b')
stairs(T,Y_markers(:,2),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,2),'g')
subplot(613)
hold on
stairs(t,X3_markers(:,3),'b')
stairs(T,Y_markers(:,3),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,3),'g')
subplot(614)
hold on
stairs(t,X3_markers(:,4),'b')
stairs(T,Y_markers(:,4),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,4),'g')
subplot(615)
hold on
stairs(t,X3_markers(:,5),'b')
stairs(T,Y_markers(:,5),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,5),'g')
subplot(616)
hold on
stairs(t,X3_markers(:,6),'b')
stairs(T,Y_markers(:,6),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,6),'g')
xlabel('Time [s]')

figure
subplot(611)
hold on
title('Cam 2 markers: red: predicted, blue: estimated by MHE, green: actual marker positions')
stairs(t,X3_markers(:,7),'b')
stairs(T,Y_markers(:,7),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,7),'g')
subplot(612)
hold on
stairs(t,X3_markers(:,8),'b')
stairs(T,Y_markers(:,8),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,8),'g')
subplot(613)
hold on
stairs(t,X3_markers(:,9),'b')
stairs(T,Y_markers(:,9),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,9),'g')
subplot(614)
hold on
stairs(t,X3_markers(:,10),'b')
stairs(T,Y_markers(:,10),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,10),'g')
subplot(615)
hold on
stairs(t,X3_markers(:,11),'b')
stairs(T,Y_markers(:,11),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,11),'g')
subplot(616)
hold on
stairs(t,X3_markers(:,12),'b')
stairs(T,Y_markers(:,12),'r.','MarkerSize',2)
stairs(t-0.2,Markers(:,12),'g')
xlabel('Time [s]')

figure
subplot(311)
hold on
title('Omega: red: predicted, blue: estimated by MHE, green: actual measurements')
plot(t,X3_imu(:,1),'b')
plot(T,Y_imu(:,1),'r.','MarkerSize',2)
plot(t,IMU(:,1),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
subplot(312)
hold on
plot(t,X3_imu(:,2),'b')
plot(T,Y_imu(:,2),'r.','MarkerSize',2)
plot(t,IMU(:,2),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
subplot(313)
hold on
plot(t,X3_imu(:,3),'b')
plot(T,Y_imu(:,3),'r.','MarkerSize',2)
plot(t,IMU(:,3),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')
ylabel('Angular velocity [rad/s]')

figure
%subplot(311)
hold on
title('Linear Acceleration in Plane Frame')
plot(t,X3_imu(:,4),'b')
plot(T,Y_imu(:,4),'r.','MarkerSize',2)
plot(t,IMU(:,4),'g')
legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(312)
%hold on
plot(t,X3_imu(:,5),'b')
plot(T,Y_imu(:,5),'r.','MarkerSize',2)
plot(t,IMU(:,5),'g')
%legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(313)
%hold on
plot(t,X3_imu(:,6),'b')
plot(T,Y_imu(:,6),'r.','MarkerSize',2)
plot(t,IMU(:,6),'g')
%legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')
ylabel('Acceleration [m/s^2]')


figure
%subplot(311)
hold on
title('Omega: red: predicted, blue: estimated by MHE, green: actual measurements')
plot(t_iterates,X_imu_iterates(:,1),'b.-')
plot(t,IMU(:,1),'g.-')
legend('MHE estimate','Actual measurement')
%subplot(312)
%hold on
plot(t_iterates,X_imu_iterates(:,2),'b.-')
plot(t,IMU(:,2),'g.-')
%legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(313)
%hold on
plot(t_iterates,X_imu_iterates(:,3),'b.-')
plot(t,IMU(:,3),'g.-')
%legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')
ylabel('Angular velocity [rad/s]')

figure
%subplot(311)
hold on
%title('Acceleration: red: predicted, blue: estimated by MHE, green: actual measurements')
plot(t_iterates,X_imu_iterates(:,4),'b.-')
plot(t,IMU(:,4),'g.-')
legend('MHE estimate','Actual measurement')
%subplot(312)
%hold on
plot(t_iterates,X_imu_iterates(:,5),'b.-')
plot(t,IMU(:,5),'g.-')
%legend('MHE estimate','MHE prediction','Actual measurement')
%subplot(313)
%hold on
plot(t_iterates,X_imu_iterates(:,6),'b.-')
plot(t,IMU(:,6),'g.-')
%legend('MHE estimate','MHE prediction','Actual measurement')
xlabel('Time [s]')
ylabel('Acceleration [m/s^2]')

% figure
% subplot(121)
% hold on
% plot(Markers(:,1),Markers(:,2),'r.','MarkerSize',2)
% plot(Markers(:,3),Markers(:,4),'g.')
% plot(Markers(:,5),Markers(:,6),'b.')
% title('Measured marker positions of Camera 1')
% subplot(122)
% hold on
% plot(X3_markers(:,1),X3_markers(:,2),'r.','MarkerSize',2)
% plot(X3_markers(:,3),X3_markers(:,4),'g.')
% plot(X3_markers(:,5),X3_markers(:,6),'b.')
% title('Estimated marker positions of Camera 1')
% 
% figure
% subplot(121)
% hold on
% plot(Markers(:,7),Markers(:,8),'r.','MarkerSize',2)
% plot(Markers(:,9),Markers(:,10),'g.')
% plot(Markers(:,11),Markers(:,12),'b.')
% title('Measured marker positions of Camera 2')
% subplot(122)
% hold on
% plot(X3_markers(:,7),X3_markers(:,8),'r.','MarkerSize',2)
% plot(X3_markers(:,9),X3_markers(:,10),'g.')
% plot(X3_markers(:,11),X3_markers(:,12),'b.')
% title('Estimated marker positions of Camera 2')
