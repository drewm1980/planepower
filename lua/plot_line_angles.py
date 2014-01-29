#!/usr/bin/env python
from scipy.io import netcdf
f = netcdf.netcdf_file('lineAngleData.nc', 'r')
from numpy import array,pi
from pylab import plot,xlabel,ylabel,show,legend,title,figure

t = array(f.variables['lineAngleSensor.data.ts_trigger'].data) # ns
t = t*1e-6 # ms

figure()
title('Compare the two clocks')
t2 = array(f.variables['TimeStamp'].data)
plot(t*1e6,t2)
xlabel('lineAngleSensor.data.ts_trigger')
ylabel('Trigger')

# Horizontal
# See lineAngleSensor component for sign documentation
angle_hor = array(f.variables['lineAngleSensor.data.angle_hor'].data) * (180.0/pi)

# Vertical
angle_ver = array(f.variables['lineAngleSensor.data.angle_ver'].data) * (180.0/pi)

f.close()

figure()
plot(t,angle_hor,t,angle_ver) 
xlabel('timestamp [ms]')
ylabel('Tether angle [Radians]')
legend(['Horizontal Angle','Vertical Angle'])
show()

