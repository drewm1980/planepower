#!/usr/bin/env python
from scipy.io import netcdf
from numpy import array, diff, pi
import pylab
from pylab import figure,plot,xlabel,ylabel,show,legend,title,subplot

from plot_line_angles import plot_line_angles
from plot_arm_speed import plot_arm_speed

if __name__=='__main__':
    figure()

    ax1 = subplot(2,1,1)
    startTime = plot_line_angles(ax1)

    ax2 = subplot(2,1,2,sharex=ax1)
    plot_arm_speed(ax2,startTime)

    show()
