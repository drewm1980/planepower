#!/usr/bin/env python

import numpy
import pickle
from pylab import plot,xlabel,ylabel,title,show

sleeptimes,latencies = pickle.load(open('latencies.pickle','rb'))

plot(sleeptimes,latencies,'k.')
xlabel('Randomly Chosen Sleep Time (ms)')
ylabel('Latency (ms)')
title('Single python instance on non-RT linux connected to two laird modules')
show()
