# -*- coding: utf-8 -*-
"""
Created on Thu Nov 24 15:59:28 2011

@author: kurt
"""
from casadi import *
import numpy as NP
import matplotlib.pyplot as plt
import time
from casadi.tools import *
from kinetics.flatkinematics import skew
from scipy.linalg.matfuncs import sqrtm
from scipy.linalg.matfuncs import toreal

H = NP.loadtxt("H.dat")
H = DMatrix(H)

X = ssym("X",22,1)
X_ref = ssym("X_ref",22,1)

f = SXFunction([],[H])
f.init()

f.generateCode("condense_data.cpp")