#!/usr/bin/env python
from scipy.io import netcdf
import numpy
from numpy import array, diff, mean
import pylab
import os
from pylab import figure,plot,xlabel,ylabel,show,legend,title

samplesToSkip = 3

f = netcdf.netcdf_file('functionGeneratorData.nc', 'r')
sentSetpoints = f.variables['functionGenerator.data.carouselSpeedSetpoint'].data[samplesToSkip+1:]
sentTimes = f.variables['functionGenerator.data.ts_trigger'].data[samplesToSkip+1:]*1e-9

f = netcdf.netcdf_file('siemensSensorsData.nc', 'r')
receivedSetpoints = f.variables['siemensSensors.data.carouselSpeedSetpoint'].data[samplesToSkip+1:]
receivedTimes = f.variables['siemensSensors.data.ts_trigger'].data[samplesToSkip+1:]*1e-9

startTime = min(sentTimes[0],receivedTimes[0])
sentTimes -= startTime;
receivedTimes -= startTime;

figure('Carousel Speed Setpoint Transmission Through the Drives')
plot(sentTimes,sentSetpoints,'b.-',receivedTimes,receivedSetpoints,'r.-')
title('Carousel Speed Setpoint Transmission Through the Drives')
ylabel('Setpoint Value [Rad/s]')
xlabel('Time [s]')
legend(['Sent','Echoed'])

#plot(xs, times / xs,'b.') 
#ylabel('Arrival Time / Time []')
#xlabel('Time [s]')


# Do an extra plot of the siemensSensors raw timestamps
#figure('Siemens PLC receive timestamp')
#title('Siemens PLC receive timestamp')

#f = netcdf.netcdf_file('siemensSensorsData.nc', 'r')
#ts_trigger = f.variables['siemensSensors.data.ts_trigger'].data[samplesToSkip+1:]*1.0e-9
#times = ts_trigger-ts_trigger[0]
#times = times[0:-1]
#xs = numpy.linspace(times[0],times[-1],len(times))

#plot(xs, times / xs,'b.') 
#ylabel('Arrival Time / Time []')
#xlabel('Time [s]')

show()
