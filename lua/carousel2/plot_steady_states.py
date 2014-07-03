#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import numpy
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title

print('loading data...')
rootName = 'lineAngleSensor2'
f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
memberName = 'azimuth'
azimuth = f.variables[rootName+'.data.'+memberName].data[1:]
memberName = 'elevation'
elevation = f.variables[rootName+'.data.'+memberName].data[1:]
ts_trigger_las = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

rootName = 'siemensSensors'
f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
setpoint = f.variables[rootName+'.data.'+'carouselSpeedSetpoint'].data[1:]
speed = f.variables[rootName+'.data.'+'carouselSpeedSmoothed'].data[1:]
ts_trigger_siemens = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

# Chose intersection of the two time ranges
startTime = max(ts_trigger_las[0],ts_trigger_siemens[0])  
endTime = min(ts_trigger_las[-1],ts_trigger_siemens[-1])
ts_trigger_las -= startTime
ts_trigger_siemens -= startTime
startTime_new = 0
endTime_new = endTime-startTime

times = len(ts_trigger_las)+len(ts_trigger_siemens)

# Since we didn't have the resampler turned on...
t = numpy.linspace(startTime_new, endTime_new, times)
elevation_resampled = numpy.interp(t, ts_trigger_las, elevation)
speed_resampled = numpy.interp(t, ts_trigger_siemens, speed)

#figure()
#title('Sanity check resampled signals for elevation')
#plot(ts_trigger_las, elevation, 'b.-', t, elevation_resampled, 'r.')

#figure()
#title('Sanity check resampled signals for speed')
#plot(ts_trigger_siemens, speed, 'b.-', t, speed_resampled, 'r.')

figure()
plot(speed_resampled, elevation_resampled,'b.-') 
title('(Hopefully) Steady-State plot')
ylabel('Line Angle Elevation [Rad]')
xlabel('Arm speed on MOTOR side of belt [Rad/s]')

show()
print('...done')
