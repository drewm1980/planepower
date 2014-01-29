#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array,pi
from pylab import plot,xlabel,ylabel,show,legend,title,figure
import matplotlib.pyplot as plt

f_lineAngle = netcdf.netcdf_file('lineAngleData.nc', 'r')
f_imu = netcdf.netcdf_file('imuData.nc', 'r')
f_camera = netcdf.netcdf_file('cameraData.nc', 'r')
f_controls = netcdf.netcdf_file('controlsData.nc', 'r')
f_encoder = netcdf.netcdf_file('encoderData.nc', 'r')

ts_imu = array(f_imu.variables['mcuHandler.data.ts_trigger'].data) # ns

plt.ion()
plt.figure()
plt.subplot(321)
plt.plot(ts_imu,array(f_imu.variables['mcuHandler.data.accl_x'].data))
plt.ylabel('acc x')
plt.subplot(323)
plt.plot(ts_imu,array(f_imu.variables['mcuHandler.data.accl_y'].data))
plt.ylabel('acc y')
plt.subplot(325)
plt.plot(ts_imu,array(f_imu.variables['mcuHandler.data.accl_z'].data))
plt.ylabel('acc z')
plt.subplot(322)
plt.plot(ts_imu,array(f_imu.variables['mcuHandler.data.gyro_x'].data))
plt.ylabel('gyro x')
plt.subplot(324)
plt.plot(ts_imu,array(f_imu.variables['mcuHandler.data.gyro_y'].data))
plt.ylabel('gyro y')
plt.subplot(326)
plt.plot(ts_imu,array(f_imu.variables['mcuHandler.data.gyro_z'].data))
plt.ylabel('gyro z')
plt.show()
