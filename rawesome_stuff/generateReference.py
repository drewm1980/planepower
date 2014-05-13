#!/usr/bin/env python

import sys, os, shutil

from rawe.models.arianne_conf import makeConf
from offline_mhe_test import carouselModel

from rawekite.carouselSteadyState import getSteadyState

import numpy as np

def generateReference(propDir, refCableLength, refSpeed):
    """
    A function for generation of reference around the steady state.
    
    For now, the goal is just to decrease Z slowly about 10 cm from the
    steady state specified by input arguments.
    """
    
    ref = []
    def stackReferencePoint( _ss ):
        ref.append( _ss )
    
    # Make conf and the DAE
    conf = makeConf()
    dae = carouselModel.makeModel(conf, propertiesDir = propDir)

    # Actuator limitations
    aileron_bound = conf['aileron_bound']
    elevator_bound = conf['elevator_bound']
    
    #
    # Configuration for steady state computation
    #

    r0 = refCableLength
    omega0 = refSpeed

    guess = {'x':r0, 'y':0, 'z':0,
             'r':r0, 'dr':0,
             'e11':0, 'e12':-1, 'e13':0,
             'e21':0, 'e22':0, 'e23':1,
             'e31':-1, 'e32':0, 'e33':0,
             'dx':0, 'dy':0, 'dz':0,
             'w_bn_b_x':0, 'w_bn_b_y':omega0, 'w_bn_b_z':0,
             'ddelta':omega0,
             'cos_delta':1, 'sin_delta':0,
             'aileron':0, 'elevator':0, 'rudder':0, 'flaps':0,
             'daileron':0, 'delevator':0, 'drudder':0, 'dflaps':0,
             'nu':100, 'motor_torque':0,
             'dmotor_torque':0, 'ddr':0,
             'dddr':0.0, 'w0':0.0}
    
    dotGuess = {'x':0, 'y':0, 'z':0, 'dx':0, 'dy':0, 'dz':0,
                'r':0, 'dr':0,
                'e11':0, 'e12':0, 'e13':0,
                'e21':0, 'e22':0, 'e23':0,
                'e31':0, 'e32':0, 'e33':0,
                'w_bn_b_x':0, 'w_bn_b_y':0, 'w_bn_b_z':0,
                'ddelta':0,
                'cos_delta':0, 'sin_delta':omega0,
                'aileron':0, 'elevator':0, 'rudder':0, 'flaps':0,
                'motor_torque':0, 'ddr':0}

    bounds = {'x':(-1.1 * r0, 1.1 * r0), 'y':(-1.1 * r0, 1.1 * r0), 'z':(-1.1 * r0, 1.1 * r0),
             'dx':(0, 0), 'dy':(0, 0), 'dz':(0, 0),
             'r':(r0, r0), 'dr':(0, 0), 'ddr':(0, 0), 'dddr':(0, 0),
             'e11':(-2, 2), 'e12':(-2, 2), 'e13':(-2, 2),
             'e21':(-2, 2), 'e22':(-2, 2), 'e23':(-2, 2),
             'e31':(-2, 2), 'e32':(-2, 2), 'e33':(-2, 2),
             'w_bn_b_x':(-100, 100), 'w_bn_b_y':(-100, 100), 'w_bn_b_z':(-100, 100),
             'ddelta':(omega0, omega0),
             'cos_delta':(1, 1), 'sin_delta':(0, 0),
             
              'aileron':(-aileron_bound, aileron_bound), 'elevator':(-elevator_bound, elevator_bound), 'rudder':(-0.2, 0.2), 'flaps':(-0.2, 0.2),
             
             'daileron':(0, 0), 'delevator':(0, 0), 'drudder':(0, 0), 'dflaps':(0, 0),
              
             
             'motor_torque':(-1000, 1000), 'dmotor_torque':(0, 0),
             
             'w0':(0, 0),
             
             'nu':(0, 3000),
             }

    dotBounds = {'x':(-1, 1), 'y':(-1, 1), 'z':(-1, 1),
                 'dx':(0, 0), 'dy':(0, 0), 'dz':(0, 0),
                 
                 'r':(-0, 0), 'dr':(0, 0), 'ddr':(0, 0),
                 
                 'e11':(-50, 50), 'e12':(-50, 50), 'e13':(-50, 50),
                 'e21':(-50, 50), 'e22':(-50, 50), 'e23':(-50, 50),
                 'e31':(-50, 50), 'e32':(-50, 50), 'e33':(-50, 50),
                 
                 'w_bn_b_x':(0, 0), 'w_bn_b_y':(0, 0), 'w_bn_b_z':(0, 0),
                 
                 'ddelta':(0, 0),
                 
                 'cos_delta':(omega0 - 10, omega0 + 10), 'sin_delta':(omega0 - 10, omega0 + 10),
                 
                 'aileron':(-1, 1), 'elevator':(-1, 1), 'rudder':(-1, 1), 'flaps':(-1, 1),
                 
                 'motor_torque':(-1000, 1000)
                 }
    
    # Generate the first reference
    pt = 1
    ss, dss = getSteadyState(dae, conf, refSpeed, refCableLength,
                             guess = guess, dotGuess = dotGuess,
                             bounds = bounds, dotBounds = dotBounds,
                             verbose = False
                             )
    stackReferencePoint( ss )
    
    print repr( pt ) + ". point of the reference trajectory is generated."
    
    ssZ = ss[ "z" ]
    
    ddz = 0.10
    
    x = np.linspace(-12, 12, num = 100)
    sszVec = ssZ - ddz * 1.0 / (1.0 + np.exp(-1.0 * x))

    for z in sszVec:
        rDict = {"z": (z, z)}
        ss, dss = getSteadyState(dae, conf, refSpeed, refCableLength, ref_dict = rDict,
                                 guess = ss, dotGuess = dss,
                                 bounds = bounds, dotBounds = dotBounds,
                                 verbose = False
                                 )
        stackReferencePoint( ss )
        pt += 1
        print repr( pt ) + ". point of the reference trajectory is generated."
        
    refOut = {}
    lenRef = None
    for name in dae.xNames() + dae.zNames() + dae.uNames() + dae.pNames():
        points = [pts[ name ] for pts in ref]
        # Go away and return back...
        refOut[ name ] = np.concatenate((points, points[::-1]), axis = 0)
        if lenRef is None:
            lenRef = refOut[ name ].shape[ 0 ]
        
    #
    # Now prepare a string with references
    #
    
    nx = len(dae.xNames())
    nu = len(dae.uNames())
    
    refCode = []
    refUpDown = ref + ref[::-1]
    for r in refUpDown:
        t = "{ {" + \
            ", ".join([repr( r[ name ] ) for name in dae.xNames()]) + \
            "}, {" + \
            ", ".join([repr( r[ name ] ) for name in dae.uNames()]) + \
            "} }"
        refCode.append( t )
    refs = ",\n".join( refCode )
    
    code = """\
#define REF_NUM_POINTS %(nRef)d

struct Reference
{
    double x[%(nx)d];
    double u[%(nu)d];
};

static const Reference references[ %(nRef)d ] =
{ 
%(refs)s
};
""" % {"nx": nx, "nu": nu, "nRef": len( refUpDown ), "refs": refs}
        
    return refOut, code

if __name__=='__main__':
    assert len(sys.argv) == 2, \
        'need to call generateReference.py with the properties directory'
    propsDir = sys.argv[1]
    
    print "This is just for fun, call generateReference function directly."
    
    ref, code = generateReference(propsDir, 2.0, -4.0)
    
    fw = open("sample_reference.cpp", "w")
    fw.write( code )
    fw.close()
    
    import matplotlib.pyplot as plt
    fig = plt.figure()
    plt.title("x, y, z")
    plt.subplot(3, 1, 1)
    plt.plot(ref["x"])
    plt.subplot(3, 1, 2)
    plt.plot(ref["y"])
    plt.subplot(3, 1, 3)
    plt.plot(ref["z"])
    
    fig = plt.figure()
    plt.title("dx, dy, dz")
    plt.subplot(3, 1, 1)
    plt.plot(ref["dx"])
    plt.subplot(3, 1, 2)
    plt.plot(ref["dy"])
    plt.subplot(3, 1, 3)
    plt.plot(ref["dz"])
    
    fig = plt.figure()
    plt.title("DCM")
    for k, name in enumerate(["e11", "e12", "e13", "e21", "e22", "e23", "e31", "e32", "e33"]):
        plt.subplot(3, 3, k + 1)
        plt.plot(ref[ name ])
        
    fig = plt.figure()
    for k, name in enumerate( ["aileron", "elevator"] ):
        plt.subplot(2, 1, k + 1)
        plt.plot(ref[ name ])
    
    plt.show()
    
    
