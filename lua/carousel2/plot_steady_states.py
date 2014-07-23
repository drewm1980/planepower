#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff
import numpy
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title
from scipy.optimize import curve_fit
from string import Template

print('WARNING!!!! This will only be something like the steady states if your LAST experiment ONLY changed the speed reference VERY slowly!')

def load_steady_state_experiments_data():
    samplesToSkip = 3

    print('loading data...')
    rootName = 'lineAngleSensor2'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    memberName = 'azimuth'
    azimuth = f.variables[rootName+'.data.'+memberName].data[samplesToSkip:]
    memberName = 'elevation'
    elevation = f.variables[rootName+'.data.'+memberName].data[samplesToSkip:]
    ts_trigger_las = f.variables[rootName+'.data.ts_trigger'].data[samplesToSkip:]*1.0e-9

    rootName = 'siemensSensors'
    f = netcdf.netcdf_file(rootName+'Data.nc', 'r')
    setpoint = f.variables[rootName+'.data.'+'carouselSpeedSetpoint'].data[samplesToSkip:]
    speed = f.variables[rootName+'.data.'+'carouselSpeedSmoothed'].data[samplesToSkip:]
    ts_trigger_siemens = f.variables[rootName+'.data.ts_trigger'].data[samplesToSkip:]*1.0e-9

    # Chose intersection of the two time ranges
    startTime = max(ts_trigger_las[0],ts_trigger_siemens[0])  
    endTime = min(ts_trigger_las[-1],ts_trigger_siemens[-1])
    ts_trigger_las -= startTime
    ts_trigger_siemens -= startTime
    startTime_new = 0
    endTime_new = endTime-startTime

    times = len(ts_trigger_las)+len(ts_trigger_siemens)

    # Since we didn't have the resampler turned on...
    t = numpy.linspace(startTime_new, endTime_new, times)
    elevation_resampled = numpy.interp(t, ts_trigger_las, elevation)
    speed_resampled = numpy.interp(t, ts_trigger_siemens, speed)
    
    return speed_resampled, elevation_resampled

def parse_constant_from_lua(constantName,fileName='experiment_helpers.lua',verbose=False):
    for line in open(fileName):
        if constantName in line:
            value = float(line .split('--')[0] .split('=')[1])
            if verbose:
                print 'Parsed out ' + constantName + ' as ' + str(value)
            return value
    raise Exception('Failed to parse ' + constName + ' out of file ' + fileName + ' !!!' )

def extract_steady_states(speed_resampled,elevation_resampled):
    takeoffAngle = parse_constant_from_lua('takeoffAngle')

    # Filter out data below flying speed
    trimmed = [(a,b) for (a,b) in zip(speed_resampled,elevation_resampled) if b>takeoffAngle]
    speed,elevation = zip(*trimmed)
    speed = numpy.array(speed)
    elevation = numpy.array(elevation)

    # Some function types for curve fitting...
    def quadratic(x, a, b, c):
        return a + b*x + c*x*x
    def cubic(x, a, b, c, d):
        return a + b*x + c*x*x + d*x*x*x
    func = cubic

    # Do polynomial fit for speed -> elevation map
    elevationCoefficients, pcov = curve_fit(func, speed, elevation)

    # Do polynomial fit for elevation -> speed map
    speedCoefficients, pcov = curve_fit(func, elevation, speed)

    # Generate lookup tables
    minSpeed = numpy.min(speed)
    maxSpeed = numpy.max(speed)
    elevationTableSpeeds = numpy.linspace(minSpeed,maxSpeed,256)
    def f_elevation(speed):
        return func(speed,*elevationCoefficients)
    elevationTableElevations = f_elevation(elevationTableSpeeds)

    minElevation = numpy.min(elevation)
    maxElevation = numpy.max(elevation)
    speedTableElevations = numpy.linspace(minElevation,maxElevation,256)
    def f_speed(speed):
        return func(speed,*speedCoefficients)
    speedTableSpeeds = f_speed(speedTableElevations)

    # yTable is a vector of length 256
    # This function will do linear interpolation into yTable given an x value
    def table_lookup(x,xMin,xMax,yTable):
        if x < xMin or x > xMax:
            raise Exception("x is out of bounds!!")
        xScaled = (x-xMin)*(1.0/xMax)*255.0 #  in [0,255.0]
        y1 = yTable[floor(xScaled)]
        y2 = yTable[ceil(xScaled)]
        y = y1 + (y2-y1) * (xScaled - floor(xScaled))
        return y

    # Codegen the two maps in matlab:

    # Codegen the two maps in C:
    lookup_template_c = Template("""
double lookup_steady_state_${raange}(double ${domain})
{
    const double ${domain}Min = ${Min};
    const double ${domain}Max = ${Max};
    const static double ${raange}Table[256] = ${theTable};
    if ((${domain} < ${domain}Min) || (${domain} > ${domain}Max)) 
    {
        cout << "WARNING!!! Value passed to table_lookup_8bit_${raange} was out of range!!!" << endl;
        return nan("");
    }
    double ${domain}Scaled = (${domain}-${domain}Min)*(1.0/${domain}Max)*255.0;
    double ${raange}1 = ${raange}Table[(int)floor(${domain}Scaled)];
    double ${raange}2 = ${raange}Table[(int)ceil(${domain}Scaled)];
    double ${raange} = ${raange}1 + (${raange}2-${raange}1) * (${domain}Scaled - floor(${domain}Scaled));
    return ${raange};
}

""")
    def vector_as_string(table):
        def group(lst, n):
          for i in range(0, len(lst), n):
            val = lst[i:i+n]
            if len(val) == n:
              yield tuple(val)
        strings = map(str,table)
        strings = [s+', ' for s in strings]
        strings[-1] = strings[-1][:-2] # Remove last comma from the last row
        rows = list(group(strings,4))
        rows = ['\t\t\t' + ''.join(row) for row in rows]
        return '\n'.join(rows)

    f_speed = lookup_template_c.substitute(domain='elevation',
                                           raange='speed',
                                           Min=minElevation,
                                           Max=maxElevation,
                                           theTable = '{\n'+vector_as_string(speedTableSpeeds)+'}')
    f_elevation = lookup_template_c.substitute(domain='speed',
                                           raange='elevation',
                                           Min=minSpeed,
                                           Max=maxSpeed,
                                           theTable = '{\n'+vector_as_string(elevationTableElevations)+'}')
    filename = 'steady_state_lookup_tables.cpp'
    boilerplate = """// Warning!!! This file was code generated by plot_steady_states.py!
// These functions perform lookups in both directions for the steady state height of the ball vs. carousel arm speed.
#include <math.h>
#include <iostream>
using namespace std;
"""
    fileContents = boilerplate + f_speed + f_elevation
    fd = open(filename,'w')
    fd.write(fileContents)
    fd.close()

    return elevationTableSpeeds,elevationTableElevations,speedTableElevations,speedTableSpeeds

if __name__=='__main__':

    speed_resampled, elevation_resampled = load_steady_state_experiments_data()

    #figure()
    #title('Sanity check resampled signals for elevation')
    #plot(ts_trigger_las, elevation, 'b.-', t, elevation_resampled, 'r.')

    #figure()
    #title('Sanity check resampled signals for speed')
    #plot(ts_trigger_siemens, speed, 'b.-', t, speed_resampled, 'r.')

    elevationTableSpeeds,elevationTableElevations,speedTableElevations,speedTableSpeeds = extract_steady_states(speed_resampled,elevation_resampled)

    figure(1)
    plot(speed_resampled, elevation_resampled/numpy.pi*180,'r.-',
         elevationTableSpeeds,elevationTableElevations/numpy.pi*180,'g.-',
         speedTableSpeeds,speedTableElevations/numpy.pi*180,'b.-') 
    title('(Hopefully) Steady-State plot')
    ylabel('Line Angle Elevation [Deg]')
    xlabel('Arm speed on MOTOR side of belt [Rad/s]')
    legend(['Measured Curve','Speed->Elevation Curve fit','Elevation->Speed Curve fit'])

    show()
