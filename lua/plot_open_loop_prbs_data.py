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

ts_lineAngle = array(f_lineAngle.variables['lineAngleSensor.data.ts_trigger'].data) # ns
ts_imu = array(f_imu.variables['mcuHandler.data.ts_trigger'].data) # ns
ts_camera = array(f_camera.variables['LEDTracker.data.ts_trigger'].data) # ns
ts_encoder = array(f_encoder.variables['encoder.data.ts_trigger'].data) # ns

plt.ion()
plt.figure()
plt.subplot(221)
plt.plot(ts_lineAngle)
plt.subplot(222)
plt.plot(ts_imu)
plt.subplot(223)
plt.plot(ts_camera)
plt.subplot(224)
plt.plot(ts_encoder)
plt.show()
t = array(f.variables['lineAngleSensor.data.ts_trigger'].data) # ns
t = t*1e-6 # ms

figure()
title('Compare the two clocks')
t2 = array(f.variables['TimeStamp'].data)
plot(t*1e6,t2)
xlabel('lineAngleSensor.data.ts_trigger')
ylabel('Trigger')

# Horizontal
# See lineAngleSensor component for sign documentation
angle_hor = array(f.variables['lineAngleSensor.data.angle_hor'].data) * (180.0/pi)

# Vertical
angle_ver = array(f.variables['lineAngleSensor.data.angle_ver'].data) * (180.0/pi)

f.close()

figure()
plot(t,angle_hor,t,angle_ver) 
xlabel('timestamp [ms]')
ylabel('Tether angle [Radians]')
legend(['Horizontal Angle','Vertical Angle'])
show()

