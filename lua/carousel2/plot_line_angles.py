#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,subplot

def plot_line_angles(axis,startTime=-1):
    rootName = 'lineAngleSensor2'

    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')

    memberName = 'azimuth'
    azimuth = f.variables[rootName+'.data.'+memberName].data[1:]
    memberName = 'elevation'
    elevation = f.variables[rootName+'.data.'+memberName].data[1:]

    ts_trigger = f.variables[rootName+'.data.ts_trigger'].data[1:]*1.0e-9

    if startTime == -1:
        startTime = ts_trigger[0]

    plot(ts_trigger-startTime, azimuth,'r.-',
         ts_trigger-startTime, elevation,'b.-') 
    xlabel('Time [s]')
    ylabel('Angle [Rad]')
    legend(['Azimuth', 'Elevation'])

    return startTime

#figure()
#plot(azimuth,elevation,'b.-') 
#xlabel('Azimuth [Rad]')
#ylabel('Elevation [Rad]')
#pylab.axis('equal')

if __name__=='__main__':
    figure()
    axis = subplot(1,1,1)
    plot_line_angles(axis,startTime=0)
    show()
