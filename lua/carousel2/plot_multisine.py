#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff, pi
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title,subplot

from plot_line_angles import plot_line_angles, plot_function_generator_reference_angle
from plot_arm_speed import plot_arm_speed, plot_function_generator_reference_speed

def plot_function_generator_cycle_clock(axis, startTime=-1):
    startSample = 3
    rootName = 'functionGenerator'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    data = f.variables[rootName+'.refData.'+'cycle'].data[startSample:]
    ts_trigger = f.variables[rootName+'.refData.ts_trigger'].data[startSample:]*1.0e-9

    if startTime == -1:
        startTime = ts_trigger[0]
    times = ts_trigger-startTime
    plot(times,data, '.-', label='FunctionGenerator Cycle Clock')
    return startTime



if __name__=='__main__':
    figure(1)
    pylab.hold(True)

    ax1 = subplot(2,1,1)
    startTime = plot_line_angles(ax1)
    legend()

    ax2 = subplot(2,1,2,sharex=ax1)
    plot_function_generator_reference_speed(ax2,startTime)
    plot_function_generator_cycle_clock(ax2,startTime)
    legend()
    show()
