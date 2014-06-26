#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title

#whichDrive = 'winch'
whichDrive = 'carousel'

rootName = 'siemensSensors'
f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
speed = f.variables[rootName+'.data.'+whichDrive+'SpeedSetpoint'].data[1:]
current = f.variables[rootName+'.data.'+whichDrive+'Current'].data[1:]
time1 = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

rootName = 'controller'
f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
command = f.variables[rootName+'.data.'+whichDrive+'SpeedSetpoint'].data[1:]
time2 = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

figure()
times = ts_trigger-ts_trigger[0]
plot(time1, speed, 'r.', time1,current,'g.', time2, command,'b.') 
ylabel('Various units')
xlabel('Time [s]')
legend(['Speed Setpoint from Drive', 'Current Measurement from Drive', 'Speed Setpoint TO drive'])
title('Response times of (digital part of) the ' + whichDrive + ' drive')

show()
print('...done')
