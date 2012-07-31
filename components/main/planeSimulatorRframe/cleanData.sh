cat state.txt | grep -v nan > state.dat
sed -i "1,3d" state.dat
cat IMU.txt | grep -v nan > IMU.dat
sed -i "1,3d" IMU.dat
cat IMUBuffered.txt | grep -v nan > IMUBuffered.dat
sed -i "1,3d" IMUBuffered.dat
