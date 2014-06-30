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

ts_trigger = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

figure()
plot(ts_trigger-ts_trigger[0], azimuth,'b.-') 
xlabel('Time [s]')
ylabel('Azimuth [Rad]')

figure()
plot(ts_trigger-ts_trigger[0], elevation,'b.-') 
ylabel('Elevation [Rad]')
xlabel('Time [s]')

figure()
plot(azimuth,elevation,'b.-') 
xlabel('Azimuth [Rad]')
ylabel('Elevation [Rad]')
pylab.axis('equal')

show()
print('...done')
