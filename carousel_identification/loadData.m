clear all

% !scp kurt@192.168.1.111:/home/kurt/planepower/usecases/collectMeasurements/imu_arm.dat imu_arm_max_speed_004.dat
% !cat imu_arm_max_speed_004.dat | grep -v nan >imu_arm_max_speedClean_004.dat
% !sed -i "1,3d" imu_arm_max_speedClean_004.dat
% 
% !scp kurt@192.168.1.111:/home/kurt/planepower/usecases/collectMeasurements/imu_arm_zero_speed.dat imu_arm_zero_speed.dat
% !cat imu_arm_zero_speed_004.dat | grep -v nan >imu_arm_zero_speedClean_004.dat
% !sed -i "1,3d" imu_arm_zero_speedClean_004.dat

data_max_speed = dlmread('imu_arm_max_speedClean_004.dat');
data_zero_speed = dlmread('imu_arm_zero_speedClean_004.dat');

data_max_speed = data_max_speed(20:end,:);
data_zero_speed = data_zero_speed(20:end,:);

t = data_max_speed(:,1);
imuStamp = data_max_speed(:,2);
imuData = data_max_speed(:,3:8);
encoderStamp = data_max_speed(:,9);
delta = data_max_speed(:,10);
ddelta = data_max_speed(:,11);

% remove duplicate rows of IMU data
imuStampClean = [imuStamp(1,:)];
imuDataClean = [imuData(1,:)];
t_clean = [t(1)];
for i=2:numel(t)
    if(not(imuStamp(i) == imuStamp(i-1)))
        imuStampClean = [imuStampClean; imuStamp(i)];
        imuDataClean = [imuDataClean; imuData(i,:)];
        t_clean = [t_clean; t(i)];
    end
end
t = t_clean;
imuStamp = imuStampClean;
imuData = imuDataClean;

% remove duplicate rows of encoder data
encoderStampClean = [encoderStamp(1,:)];
deltaClean = [delta(1,:)];
ddeltaClean = [ddelta(1,:)];
for i=2:numel(encoderStamp)
    if(not(encoderStamp(i) == encoderStamp(i-1)))
        encoderStampClean = [encoderStampClean; encoderStamp(i)];
        deltaClean = [deltaClean; delta(i,:)];
        ddeltaClean = [ddeltaClean; ddelta(i,:)];
    end
end
encoderStamp = encoderStampClean;
delta = deltaClean;
ddelta = ddeltaClean;

% Align encoder data with imu data
data_aligned = [t imuData nan(numel(t),2)];
for i=1:numel(imuStamp)
    for j=max(1,i-5):min(i+5,numel(encoderStamp))
        if imuStamp(i) == encoderStamp(j)
            data_aligned(i,8) = delta(j);
            data_aligned(i,9) = ddelta(j);
            break;
        end
    end
end
% Now there might be some NaNs left at start or end of data_aligned
% because imu and encoder data might be starting/ending at slightly
% different times
i_start = 1;
while isnan(data_aligned(i_start,8))
    i_start = i_start+1;
end
i_end = 0;
while isnan(data_aligned(end-i_end,8))
    i_end = i_end+1;
end
data_aligned = data_aligned(i_start:end-i_end,:);


% Do the same for the data at zero speed
t_zero = data_zero_speed(:,1);
imuStamp_zero = data_zero_speed(:,2);
imuData_zero = data_zero_speed(:,3:8);
encoderStamp_zero = data_zero_speed(:,9);
delta_zero = data_zero_speed(:,10);
ddelta_zero = data_zero_speed(:,11);

% remove duplicate rows of IMU data
imuStamp_zeroClean = [imuStamp_zero(1,:)];
imuData_zeroClean = [imuData_zero(1,:)];
t_zero_clean = [t_zero(1)];
for i=2:numel(t_zero)
    if(not(imuStamp_zero(i) == imuStamp_zero(i-1)))
        imuStamp_zeroClean = [imuStamp_zeroClean; imuStamp_zero(i)];
        imuData_zeroClean = [imuData_zeroClean; imuData_zero(i,:)];
        t_zero_clean = [t_zero_clean; t_zero(i)];
    end
end
t_zero = t_zero_clean;
imuStamp_zero = imuStamp_zeroClean;
imuData_zero = imuData_zeroClean;

% remove duplicate rows of encoder data
encoderStamp_zeroClean = [encoderStamp_zero(1,:)];
delta_zeroClean = [delta_zero(1,:)];
ddelta_zeroClean = [ddelta_zero(1,:)];
for i=2:numel(encoderStamp_zero)
    if(not(encoderStamp_zero(i) == encoderStamp_zero(i-1)))
        encoderStamp_zeroClean = [encoderStamp_zeroClean; encoderStamp_zero(i)];
        delta_zeroClean = [ddelta_zeroClean; delta_zero(i,:)];
        ddelta_zeroClean = [ddelta_zeroClean; ddelta_zero(i,:)];
    end
end
encoderStamp_zero = encoderStamp_zeroClean;
delta_zero = delta_zeroClean;
ddelta_zero = ddelta_zeroClean;

% Align encoder data with imu data
data_aligned_zero = [t_zero imuData_zero nan(numel(t_zero),2)];
for i=1:numel(imuStamp_zero)
    for j=max(1,i-5):min(i+5,numel(encoderStamp_zero))
        if imuStamp_zero(i) == encoderStamp_zero(j)
            data_aligned_zero(i,8) = delta_zero(j);
            data_aligned_zero(i,9) = ddelta_zero(j);
            break;
        end
    end
end
% Now there might be some NaNs left at start or end of data_aligned
% because imu and encoder data might be starting/ending at slightly
% different times
i_start = 1;
while isnan(data_aligned_zero(i_start,8))
    i_start = i_start+1;
end
i_end = 0;
while isnan(data_aligned_zero(end-i_end,8))
    i_end = i_end+1;
end
data_aligned_zero = data_aligned_zero(i_start:end-i_end,:);

