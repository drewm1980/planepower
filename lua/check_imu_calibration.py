#!/usr/bin/env python
from scipy.io import netcdf
f = netcdf.netcdf_file('imuData.nc', 'r')
from numpy import array,pi,hstack,vstack,sqrt
from pylab import plot,xlabel,ylabel,show,legend

t = array(f.variables['mcuHandler.data.ts_trigger'].data) # ns
t = t*1e-6 # ms

accl_x = array(f.variables['mcuHandler.data.accl_x'].data)
accl_y = array(f.variables['mcuHandler.data.accl_y'].data)
accl_z = array(f.variables['mcuHandler.data.accl_z'].data)
gyro_x = array(f.variables['mcuHandler.data.gyro_x'].data)
gyro_y = array(f.variables['mcuHandler.data.gyro_y'].data)
gyro_z = array(f.variables['mcuHandler.data.gyro_z'].data)
accl =  vstack((accl_x,accl_y,accl_z))
magnitudes = sqrt((accl*accl).sum(axis=0))

f.close()
plot(t,magnitudes) 
xlabel('timestamp [ms]')
ylabel('magnitude of IMU acceleration [m/s^2]')
#legend(['Horizontal Angle','Vertical Angle'])
show()

