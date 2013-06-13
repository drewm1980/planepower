from scipy.io import netcdf
f = netcdf.netcdf_file('angleData.nc', 'r')
from numpy import array
from pylab import plot,xlabel,ylabel,show 

t = array(f.variables['lineAngleSensor.timeStamps.0'].data)
v1 = array(f.variables['lineAngleSensor.voltages.0'].data)
v2 = array(f.variables['lineAngleSensor.voltages.1'].data)
f.close()
plot(t,v1) 
xlabel('timestamp')
ylabel('v1 [V]')
show()

