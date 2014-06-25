#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff, mean
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title

print('loading data...')

print('loading timing data to plot jitter...')

fileRoots = ['controller', 'resampler', 'lineAngleSensor2', 'siemensSensors']

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

show()
