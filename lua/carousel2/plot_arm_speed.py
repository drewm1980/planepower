#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff, pi
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title,subplot

startSample = 5

def plot_function_generator_reference_speed(axis, startTime=-1):
    rootName = 'functionGenerator'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    data3 = f.variables[rootName+'.data.'+'carouselSpeedSetpoint'].data[startSample:]
    ts_trigger3 = f.variables[rootName+'.data.ts_trigger'].data[startSample:]*1.0e-9

    if startTime == -1:
        startTime = ts_trigger3[0]

    times2 = ts_trigger3-startTime

    plot(times2,data3, 'g.-', label='FunctionGenerator Speed Setpoint (Sent)')

    return startTime


def plot_arm_speed(axis, startTime=-1):
    rootName = 'siemensSensors'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    data1 = f.variables[rootName+'.data.'+'carouselSpeedSetpoint'].data[startSample:]
    data2 = f.variables[rootName+'.data.'+'carouselSpeedSmoothed'].data[startSample:]
    ts_trigger = f.variables[rootName+'.data.ts_trigger'].data[startSample:]*1.0e-9

    # Load the actual arm speed from the arm gyro
    rootName = 'armboneLisaSensors'
    fiile = netcdf.netcdf_file(rootName+'Data.nc', 'r')

    rawdata4 = fiile.variables['armboneLisaSensors.GyroState.gr'].data[startSample:]
    ts_trigger4 = fiile.variables['armboneLisaSensors.GyroState.ts_trigger'].data[startSample:]*1.0e-9
    #fullscale = 2000 # deg/sec
    #data4 = -1.0 * rawdata4 / (2**15) * fullscale * pi/180 - 0.0202 # Rad/s
    data4 = rawdata4

    if startTime == -1:
        startTime = ts_trigger[0]

    times = ts_trigger-startTime
    times4 = ts_trigger4-startTime

    pylab.hold(True)

    plot(times, data2, '.-', label='On Motor Side of Belt')
    plot(times4, data4,'.-',  label='From Gyro on Arm')
    plot(times, data1, '.-', label='Setpoint (Echoed)')
    ylabel('Arm rotation speed [Rad/s]')
    xlabel('Time [s]')
    #legend(['Setpoint (Echoed)', 'Setpoint (Sent)', 'On Motor Side of Belt', 'From Gyro on Arm'])
    title('Plot of Signals Related to Arm Speed')
    return startTime

if __name__=='__main__':
    fig = figure()
    axis = subplot(1,1,1)
    plot_arm_speed(axis)
    legend()
    show()
    print('...done')
