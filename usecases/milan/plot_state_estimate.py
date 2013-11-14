#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
from pylab import figure,plot,xlabel,ylabel,show,legend

print('loading data...')
f = netcdf.netcdf_file('dynamicMheData.nc', 'r')
xhat1 = f.variables['mhe.stateEstimate.x_hat.0'].data
xhat2 = f.variables['mhe.stateEstimate.x_hat.1'].data
xhat3 = f.variables['mhe.stateEstimate.x_hat.2'].data
#markers = []

print('plotting...')
figure()
plot(xhat1[20:-20],'b.') 
xlabel('sample index')
ylabel('x [m]')

print('plotting...')
figure()
plot(xhat2[20:-20],'b.') 
xlabel('sample index')
ylabel('y[m]')

print('plotting...')
figure()
plot(xhat3[20:-20],'b.') 
xlabel('sample index')
ylabel('z[m]')


show()
print('...done')
