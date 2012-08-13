%close all

%!scp kurt@192.168.1.111:/home/kurt/planepower/usecases/controlExperiments/imu_controlSequence.dat imu_controlSequence.dat
%!cat imu_controlSequence.dat | grep -v nan > imu_controlSequenceClean.dat
%!sed -i "1,3d" imu_controlSequenceClean.dat


data = dlmread('imu_controlSequenceClean.dat');

for i =1:1 %just so that I can make this block small.
    NU = 3;
    NY_IMU = 6;
    scale_ur = 1.25e6;
    scale_up = 2e5;
    t_imu = data(:,1);
    IMU = data(:,2:2+NY_IMU-1);
    U = data(:,2+NY_IMU:2+NY_IMU+NU-1);
    ur = U(:,1);
    up = U(:,3);
    ur_scaled = ur/scale_ur;
    up_scaled = up/scale_up;
    w1 = IMU(:,1);
    w2 = IMU(:,2);
    w3 = IMU(:,3);
    a1 = IMU(:,4);
    a2 = IMU(:,5);
    a3 = IMU(:,6);
end
