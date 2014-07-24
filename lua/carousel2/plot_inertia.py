#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff, pi
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title,subplot,grid

def plot_arm_speeds(axis):
    startSample = 3
    rootName = 'siemensSensors'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    data1 = f.variables[rootName+'.data.'+'carouselSpeedSmoothed'].data[startSample:]
    data2 = f.variables[rootName+'.data.'+'carouselTorque'].data[startSample:]
    ts_trigger1 = f.variables[rootName+'.data.ts_trigger'].data[startSample:]*1.0e-9


    # Load the actual arm speed from the arm gyro
    rootName = 'armboneLisaSensors'
    fiile = netcdf.netcdf_file(rootName+'Data.nc', 'r')

    data3 = fiile.variables['armboneLisaSensors.GyroState.gr'].data[startSample:]
    ts_trigger3 = fiile.variables['armboneLisaSensors.GyroState.ts_trigger'].data[startSample:]*1.0e-9
    #fullscale = 2000 # deg/sec
    #data4 = -1.0 * rawdata4 / (2**15) * fullscale * pi/180 - 0.0202 # Rad/s

    times1 = ts_trigger1-ts_trigger1[0]
    times2 = ts_trigger1-ts_trigger1[0]
    times3 = ts_trigger3-ts_trigger1[0]
    plot(times1, data1, 'r.-',
         times2, data2 / 100, 'g.-',
         times3, data3,'m.-') 
    ylabel('Arm rotation speed [Rad/s]')
    xlabel('Time [s]')
    legend(['motorSpeed', 'armTorque', 'armSpeed'])
    title('Plot of Signals Related to Arm Speed')
    grid()
fig = figure()
axis = subplot(1,1,1)
plot_arm_speeds(axis)
show()
print('...done')
