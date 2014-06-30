#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title

print('loading data...')

rootName = 'siemensSensors'
f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
data1 = f.variables[rootName+'.data.'+'carouselSpeedSetpoint'].data[1:]
data2 = f.variables[rootName+'.data.'+'carouselSpeedSmoothed'].data[1:]
ts_trigger = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

rootName = 'functionGenerator'
f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
data3 = f.variables[rootName+'.data.'+'carouselSpeedSetpoint'].data[1:]
ts_trigger3 = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

figure()
times = ts_trigger-ts_trigger[0]
times2 = ts_trigger3-ts_trigger[0]
plot(times, data1, 'r.-',
     times2,data3, 'g.-',
     times, data2,'b.-') 
ylabel('Arm rotation speed [Rad/s]')
xlabel('Time [s]')
legend(['Setpoint (Echoed)', 'Setpoint (Sent)', 'Actual'])
title('Plot of Real and Acutal Arm Rotation Speed')

show()
print('...done')
