#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
from pylab import figure,plot,xlabel,ylabel,show,legend

print('loading data...')
f = netcdf.netcdf_file('imuTest.nc', 'r')
markers = []
accl_x = f.variables['mcuHandler.data.accl_x'].data
accl_y = f.variables['mcuHandler.data.accl_y'].data
accl_z = f.variables['mcuHandler.data.accl_z'].data
f.close()
print('...done')

print('plotting...')
figure()
plot(accl_x,'r.') 
plot(accl_y,'g.') 
plot(accl_z,'b.') 
xlabel('sample index')
ylabel('accelerometer data [m/s^2]')
legend(['x','y','z'])
show()
print('...done')
