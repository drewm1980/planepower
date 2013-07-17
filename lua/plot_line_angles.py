#!/usr/bin/env python
from scipy.io import netcdf
f = netcdf.netcdf_file('angleData.nc', 'r')
from numpy import array
from pylab import plot,xlabel,ylabel,show,legend

t = array(f.variables['lineAngleSensor.timeStamps.0'].data) # ns
t = t*1e-6 # ms

# Note, I manually calibrate the tether angle sensor
# such that 0V in both angles corresponds to the tether
# straight out.

# Horizontal, positive is forward
hslope = 0.4246 # V / degrees
hzero = 0.0 # volts
v1 = array(f.variables['lineAngleSensor.voltages.0'].data)
v1 = (v1-hzero)/hslope

# Vertical, positive is up
vslope = 0.6074 # V / degrees 
vzero = 0.0 # V
v2 = array(f.variables['lineAngleSensor.voltages.1'].data)
v2 = (v2-vzero)/vslope

f.close()
plot(t,v1,t,v2) 
xlabel('timestamp [ms]')
ylabel('Tether angle [Degrees]')
legend(['Horizontal Angle, + is forward','Vertical Angle, + is up'])
show()

