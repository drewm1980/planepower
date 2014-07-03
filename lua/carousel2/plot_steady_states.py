#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend

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

startTime = min(ts_trigger_las[0],ts_trigger_siemens[0]) 
ts_trigger_las -= startTime
ts_trigger_siemens -= startTime

# NEED TO RESAMPLE THE SIGNALS!!!

#figure()
#plot(ts_trigger_siemens, n,'b.-') 
#ylabel('Elevation [Rad]')
#xlabel('Time [s]')


show()
print('...done')
