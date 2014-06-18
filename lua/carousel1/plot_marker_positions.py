#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
from pylab import figure,plot,xlabel,ylabel,show,legend

print('loading data...')
f = netcdf.netcdf_file('cameraData.nc', 'r')
markers = []

for i in range(12):
    markers.append(array(f.variables['LEDTracker.markerPositionsAndCovariance.' + str(i)].data))
f.close()
print('...done')

#print('plotting...')
#figure()
#plot(frameArrivalTimeStamp[1:-3],'b.') 
#xlabel('sample index')
#ylabel('frameArrivalTimeStamp[ms]')
#legend([''])

#figure()
#plot(interframe,'b.') 
#xlabel('sample index')
#ylabel('Difference in frame arrival times [ms]')
#legend(['Interframe arrval times'])

show()
print('...done')
