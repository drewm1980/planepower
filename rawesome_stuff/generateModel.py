import rawe
import casadi as C
from casadi import *
import numpy

if __name__=='__main__':
    print "reading config..."
    conf = rawe.config.readConfig('config.ini','configspec.ini')
    
    print "creating model..."
    dae = rawe.models.carousel(conf)
    
    modelFile = dae.acadoModelGen()
    
    print "saving model..."
    f = open('out/model.cpp','w')
    f.write(modelFile)
    f.close()
