#!/usr/bin/env python
from scipy.io import netcdf
f = netcdf.netcdf_file('cameraData.nc', 'r')
from numpy import array, diff
from pylab import figure,plot,xlabel,ylabel,show,legend

print('loading data...')
triggerTimeStamp = array(f.variables['LEDTracker.timeStamps.0'].data)*1e-6 # ms
frameArrivalTimeStamp = array(f.variables['LEDTracker.timeStamps.1'].data)*1e-6 # ms
computationCompleteTimeStamp = array(f.variables['LEDTracker.timeStamps.2'].data)*1e-6 # ms
exitTimeStamp = array(f.variables['LEDTracker.timeStamps.3'].data)*1e-6 # ms
f.close()
print('...done')

interframe = diff(frameArrivalTimeStamp)[4:-4]
from numpy import median
print('median inter-frame arrival time = ' + str(median(interframe)) + ' ms')
sec = median(interframe) * 1e-3
print('median framerate = ' + str(median(1/sec)) + ' hz')

triggerToArrival=(frameArrivalTimeStamp-triggerTimeStamp)[4:-4]
print('median time for frame to arrive = ' + str(median(triggerToArrival)) + ' ms')
print('min time for frame to arrive = ' + str(min(triggerToArrival)) + ' ms')

computationTime=(computationCompleteTimeStamp - frameArrivalTimeStamp)[4:-4]
print('median time for computation= ' + str(median(computationTime)) + ' ms')

latency=(computationCompleteTimeStamp - triggerTimeStamp)[4:-4]
maxlatency = max(latency)
print('max latency, including computation = ' + str(maxlatency) + ' ms')

print('Based on latency, recommended frame rate for 1 sample delay is: ' + str(1/(maxlatency*1e-3)) + ' hz')

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
