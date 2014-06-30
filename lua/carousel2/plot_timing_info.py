#!/usr/bin/env python
from scipy.io import netcdf
import numpy
from numpy import array, diff, mean
import pylab
import os
from pylab import figure,plot,xlabel,ylabel,show,legend,title

print('loading data...')

print('loading timing data to plot jitter...')

fileRoots = ['controller', 'resampler', 'lineAngleSensor2', 'siemensSensors']
fileRoots = [root for root in fileRoots if os.path.isfile(root+'Data.nc')]

if fileRoots==[]:
    print 'Error, none of the .nc files are there!'
    exit(-1)

samplesToSkip = 3

figure('Jitter, based on diff of timestamps')
for i in range(len(fileRoots)):
    if i==0:
        title('Component startHooks jitter')
    f = netcdf.netcdf_file(fileRoots[i]+'Data.nc', 'r')
    ts_trigger = f.variables[fileRoots[i]+'.data.ts_trigger'].data[samplesToSkip+1:]*1.0e-9
    pylab.subplot(len(fileRoots),1,i+1)
    if len(ts_trigger > 0): # controller may not have run
        jitter = diff(ts_trigger) * 1e3 # ms
        jitter = jitter - mean(jitter)
        times = ts_trigger-ts_trigger[0]
        plot(times[:-1], jitter ,'b.') 
    ylabel(fileRoots[i]+' jitter [ms]')
    if i==len(fileRoots)-1:
        xlabel('Time [s]')
    print "Max jitter for "+fileRoots[i]+" is "+str(numpy.max(jitter))
    print "Mean jitter for "+fileRoots[i]+" is "+str(numpy.mean(jitter))
    print "Median jitter for "+fileRoots[i]+" is "+str(numpy.median(jitter))

# Do an extra plot of the siemensSensors raw timestamps
figure('Siemens PLC receive timestamp')
title('Siemens PLC receive timestamp')
f = netcdf.netcdf_file('siemensSensorsData.nc', 'r')
ts_trigger = f.variables['siemensSensors.data.ts_trigger'].data[samplesToSkip+1:]*1.0e-9
if len(ts_trigger > 0): # controller may not have run
    times = ts_trigger-ts_trigger[0]
    times = times[0:-1]
    xs = numpy.linspace(times[0],times[-1],len(times))
    plot(xs, times / xs,'b.') 
    ylabel('Arrival Time / Time []')
    xlabel('Time [s]')

show()
