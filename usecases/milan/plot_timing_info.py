#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
from pylab import figure,plot,xlabel,ylabel,show,legend

print('loading data...')
f = netcdf.netcdf_file('dynamicMheData.nc', 'r')
exec_fdb = f.variables['mhe.debugData.exec_fdb'].data
exec_prep = f.variables['mhe.debugData.exec_prep'].data
#markers = []

print('plotting...')
figure()
plot(exec_prep[1:-3],'b.') 
xlabel('sample index')
ylabel('Preperation step time [s]')
legend([''])

print('plotting...')
figure()
plot(exec_fdb[1:-3],'b.') 
xlabel('sample index')
ylabel('Feedback step time [s]')
legend([''])

show()
print('...done')
