#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,subplot
from plot_line_angles import plot_line_angles

def plot_pid_gains(axis, startTime=-1):
    startSample = 3
    rootName = 'controller'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    pdata = f.variables[rootName+'.debug.'+'Kp'].data[startSample:]
    idata = f.variables[rootName+'.debug.'+'Ki'].data[startSample:]
    ddata = f.variables[rootName+'.debug.'+'Kd'].data[startSample:]
    ts_trigger = f.variables[rootName+'.debug.ts_trigger'].data[startSample:]*1.0e-9

    if startTime == -1:
        startTime = ts_trigger[0]

    times2 = ts_trigger-startTime

    plot(times2,pdata, '.-', label='Proportional Gain')
    plot(times2,idata, '.-', label='Integral Gain')
    plot(times2,ddata, '.-', label='Derivative Gain')
    ylabel('PID Controller Gains')
    xlabel('Time [s]')

    return startTime

def plot_pid_errors(axis, startTime=-1):
    startSample = 3
    rootName = 'controller'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    ierror = f.variables[rootName+'.debug.'+'ierror'].data[startSample:]
    derror = f.variables[rootName+'.debug.'+'derror'].data[startSample:]
    ddata = f.variables[rootName+'.debug.'+'Kd'].data[startSample:]
    ts_trigger = f.variables[rootName+'.debug.ts_trigger'].data[startSample:]*1.0e-9

    if startTime == -1:
        startTime = ts_trigger[0]

    times2 = ts_trigger-startTime

    plot(times2,ierror, '.-', label='Integral Error')
    plot(times2,derror, '.-', label='Derivative Error')
    ylabel('PID error signals')
    xlabel('Time [s]')

    return startTime

def plot_elevation_derivative(axis, startTime=-1):
    startSample = 3
    rootName = 'controller'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    data = f.variables[rootName+'.debug.'+'derivativeLowpassFilterState'].data[startSample:]
    ts_trigger = f.variables[rootName+'.debug.ts_trigger'].data[startSample:]*1.0e-9

    if startTime == -1:
        startTime = ts_trigger[0]

    times2 = ts_trigger-startTime

    plot(times2,data, '.-', label='Smoothed derivative of elevation')
    ylabel('Elevation Derivative [Rad / s]')
    xlabel('Time [s]')

    return startTime

if __name__=='__main__':
    figure(1)
    pylab.hold(True)

    numplots = 5
    currentPlot = 0

    currentPlot = currentPlot + 1
    ax1 = subplot(numplots,1,currentPlot)
    startTime = plot_line_angles(ax1)
    legend()

    currentPlot = currentPlot + 1
    ax4 = subplot(numplots,1,currentPlot,sharex=ax1)
    plot_pid_errors(ax3,startTime)
    #legend()

    currentPlot = currentPlot + 1
    ax5 = subplot(numplots,1,4,sharex=ax1)
    plot_pid_gains(ax3,startTime)
    legend()


    show()
