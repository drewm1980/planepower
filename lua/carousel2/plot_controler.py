#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
from pylab import figure,plot,xlabel,ylabel,show,legend

print('loading data...')
f = netcdf.netcdf_file('controllerData.nc', 'r')
winchSpeedReference = f.variables['controller.data.winchSpeedReference'].data[1:]
carouselSpeedReference = f.variables['controller.data.carouselSpeedReference'].data[1:]
ts_trigger = f.variables['controller.data.ts_trigger'].data[1:]*1.0e-9

#print('plotting...')
#figure()
#plot(ts_trigger-ts_trigger[0], winchSpeedReference ,'b.') 
#xlabel('Time [s]')
#ylabel('winchSpeedReference [m/s]')
#legend([''])

print('plotting...')
figure()
plot(ts_trigger-ts_trigger[0], carouselSpeedReference ,'b.') 
xlabel('Time [s]')
ylabel('carouselSpeedReference [Rad/s]')
legend([''])

show()
print('...done')
