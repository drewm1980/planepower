#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,subplot
from plot_line_angles import plot_line_angles

def plot_elevation_derivative(axis, startTime=-1):
    startSample = 3
    rootName = 'controller'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    data = f.variables[rootName+'.debug.'+'derivativeLowpassFilterState'].data[startSample:]
    ts_trigger = f.variables[rootName+'.debug.ts_trigger'].data[startSample:]*1.0e-9

    if startTime == -1:
        startTime = ts_trigger[0]

    times2 = ts_trigger-startTime

    plot(times2,data, 'g.-', label='Smoothed derivative of elevation')
    ylabel('Elevation Derivative [Rad / s]')
    xlabel('Time [s]')

    return startTime

if __name__=='__main__':
    figure(1)
    pylab.hold(True)

    ax1 = subplot(2,1,1)
    startTime = plot_line_angles(ax1)
    legend()

    ax2 = subplot(2,1,2,sharex=ax1)
    plot_elevation_derivative(ax2,startTime)
    legend()

    show()
