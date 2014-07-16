#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import numpy
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend

print('loading data...')
rootName = 'armboneLisaSensors'

fiile = netcdf.netcdf_file(rootName+'Data.nc', 'r')

sensors = ['Gyro','Accel','Mag']
Gyro_fields = ['gp_raw','gq_raw','gr_raw']
Accel_fields = ['ax_raw','ay_raw','az_raw']
Mag_fields = ['mx_raw','my_raw','mz_raw']
s_fields = {'Gyro':Gyro_fields,
            'Accel':Accel_fields,
            'Mag':Mag_fields}

# Load all the data
startSample = 3
timebases = {}
datas = {}
startTimes = {}
for s in sensors:
    datas[s] = dict()
    timebases[s] = fiile.variables[rootName+'.'+s+'State.ts_trigger'].data[startSample:]*1.0e-9
    startTimes[s] = timebases[s][0]
    for field in s_fields[s]:
        datas[s][field] = fiile.variables[rootName+'.'+s+'State.'+field].data[startSample:]

# Remove offset from the timebases
startTime = numpy.min([startTimes[s] for s in sensors])
for s in sensors:
    timebases[s] = timebases[s] - startTime

# Time to plot!!!
yaxes = ['Rotation Rate [Rad/s]', 'Linear Acceleration [m/s/s]', 'Magnetic Field Strength [Gauss?]']
styles = ['r.-', 'g.-', 'b.-']
titles = ['Arm Mounted Gyroscope Data', 
          'Arm Mounted Accelerometer Data',
          'Arm Mounted Magnetometer Data']
legends = ['x','y','z']
for i in range(len(sensors)):
    s = sensors[i]
    figure(titles[i])
    datass = [datas[s][f] for f in s_fields[s] ]
    timebasess = (timebases[s],timebases[s],timebases[s])
    arrgs = zip(timebasess,datass,styles)
    argss = [item for subtuple in arrgs for item in subtuple]
    plot(*argss)
    xlabel('Time [s]')
    ylabel(yaxes[i])
    legend(legends)

show()

#figure()
#plot(azimuth,elevation,'b.-') 
#xlabel('Azimuth [Rad]')
#ylabel('Elevation [Rad]')
#pylab.axis('equal')

