#!/usr/bin/env python
from scipy.io import netcdf
f = netcdf.netcdf_file('lineAngleData.nc', 'r')
from numpy import array,pi
from pylab import plot,xlabel,ylabel,show,legend

t = array(f.variables['lineAngleSensor.data.ts_trigger'].data) # ns
t = t*1e-6 # ms

# Horizontal
# See lineAngleSensor component for sign documentation
angle_hor = array(f.variables['lineAngleSensor.data.angle_hor'].data) * (180.0/pi)

# Vertical
angle_ver = array(f.variables['lineAngleSensor.data.angle_ver'].data) * (180.0/pi)

f.close()
plot(t,angle_hor,t,angle_ver) 
xlabel('timestamp [ms]')
ylabel('Tether angle [Radians]')
legend(['Horizontal Angle','Vertical Angle'])
show()

