#!/usr/bin/env python

import sys, os, shutil
import numpy as np

from rawe.ocp.Ocp import generateProto

from offline_mhe_test import NMPC

from rawekite.carouselSteadyState import getSteadyState

def generateReference(nmpc, conf, refCableLength, refSpeed, visualize = False):
    """
    A function for generation of reference around the steady state.
    
    For now, the goal is just to decrease Z slowly about <ddz> from the
    steady state specified by input arguments.
    """

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

    #
    # Generate the reference
    #
    ref = []
    
    # Generate the first reference
    pt = 1
    ss, dss = getSteadyState(nmpc.dae, conf, refSpeed, refCableLength,
                             guess = guess, dotGuess = dotGuess,
                             bounds = bounds, dotBounds = dotBounds,
                             verbose = False
                             )
    ref.append( ss )
    
    print repr( pt ) + ". point of the reference trajectory is generated."
    
    ssZ = ss[ "z" ]
    
    ddz = 0.025 # [m]
    x = np.linspace(-12 * 2, 12 * 2, num = 75)
    sszVec = ssZ - ddz * 1.0 / (1.0 + np.exp(-1.0 * x))
    for z in sszVec:
        rDict = {"z": (z, z)}
        ss, dss = getSteadyState(nmpc.dae, conf, refSpeed, refCableLength, ref_dict = rDict,
                                 guess = ss, dotGuess = dss,
                                 bounds = bounds, dotBounds = dotBounds,
                                 verbose = False
                                 )
        ref.append( ss )
        pt += 1
        print repr( pt ) + ". point of the reference trajectory is generated."
        
    refOut = {}
    for name in nmpc.dae.xNames() + nmpc.dae.zNames() + nmpc.dae.uNames() + nmpc.dae.pNames():
        points = [pts[ name ] for pts in ref]
        # Go away and return back...
        refOut[ name ] = np.concatenate((points, points[::-1]), axis = 0)
    refUpDown = ref + ref[::-1]
    
    #
    # Now prepare a string with references
    #
    
    import casadi as C
    yxFun = C.SXFunction([nmpc.dae.xVec()], [C.densify(nmpc.yx)])
    yuFun = C.SXFunction([nmpc.dae.uVec()], [C.densify(nmpc.yu)])
    yxFun.init()
    yuFun.init()
    
    refCode = []
    for r in refUpDown:
        _x = [r[ name ] for name in nmpc.dae.xNames()]
        _u = [r[ name ] for name in nmpc.dae.uNames()]
        
        yxFun.setInput(_x, 0)
        yxFun.evaluate()  
        
        yuFun.setInput(_u, 0)
        yuFun.evaluate()  
        
        _r = np.concatenate((np.squeeze(np.array( yxFun.output( 0 ) )),
                             np.squeeze(np.array( yuFun.output( 0 ) ))),
                             axis = 0)
        
        t = "{" + ", ".join([repr( el ) for el in _r]) + "}"
        refCode.append( t )
    
    refs = ",\n".join( refCode )
    
    code = """\
#define REF_NUM_POINTS %(nRef)d

static const double references[ %(nRef)d ][ %(yDim)d ] =
{ 
%(refs)s
};
""" % {"yDim": nmpc.yx.shape[ 0 ] + nmpc.yu.shape[ 0 ],
       "nRef": len( refUpDown ),
       "refs": refs}

    if visualize is True:
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
    
    return refOut, code

if __name__=='__main__':
    assert len(sys.argv) == 2 or len(sys.argv) == 3, \
        'need to call generateNmpc.py with the properties directory'
    propsDir = sys.argv[1]

    nmpc, conf = NMPC.makeNmpc(propertiesDir=propsDir)

    #
    # NMPC export options
    # 

    # Sampling time
    Ts = 0.04 # [sec]
    # Reference cable length
    measCableLength = 1.7 # [m]
    # Reference speed
    steadyStateSpeed = -(55.0 / 60. * 2. * np.pi) # [rad]
    
    #
    # Generate protobuf specs for the NMPC
    #
    fw = open("DynamicMpcTelemetry.proto", "w")
    fw.write( generateProto(nmpc, "DynamicMpc") )
    fw.close()
    
    if len(sys.argv) == 3 and sys.argv[ 2 ] == "proto_only":
        print "NMPC protobuf is generated and I am out..."
        sys.exit( 0 )
    
    cgOptions= {'CXX':'clang++', 'CC':'clang',
                'CXXFLAGS':'-O3 -fPIC -finline-functions -march=native',
                'CFLAGS':'-O3 -fPIC -finline-functions -march=native',
                'hideSymbols':True}
    exportpath =  nmpc.exportCode(NMPC.mpcOpts, NMPC.mpcIntOpts, cgOptions, {})

    # Copy the library and the headers to output location
    for filename in ['acado_common.h', 'acado_qpoases_interface.hpp', 'ocp.o']:
        fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)
    
    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
    
    # Generate a data file with weights
    fw = open("mpc_configuration.h", "w")
    fw.write(
'''
#ifndef NMPC_CONFIGURATION
#define NMPC_CONFIGURATION

'''
            )

    fw.write("// This file was created from the file: " + os.path.realpath(__file__) + "\n")
    
    fw.write("#define mpc_sampling_time " + repr( Ts ) + "\n\n\n");
    
    xuNames = nmpc.yxNames + nmpc.yuNames
    fw.write("const double mpc_weights[ " + str( len( xuNames ) ) + " ] =\n{\n")
    for name in xuNames:
        fw.write("\t" + repr( NMPC.mpcWeights[ name ] ) + ", // " + name + "\n")
    fw.write("};\n\n")
    
    fw.write("// Differential variables\n")
    for k, name in enumerate( nmpc.dae.xNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    fw.write("\n\n")
    
    fw.write("// Control variables\n")
    for k, name in enumerate( nmpc.dae.uNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    fw.write("\n\n")
    
    # Output offsets for references
    fw.write("// References offsets\n")
    yOffset = 0
    for name in nmpc.yxNames:
        fw.write("#define offset_" + str( name ) + " " + str( yOffset ) + "\n")
        yOffset += nmpc[ name ].shape[ 0 ]
    for name in nmpc.yuNames:
        fw.write("#define offset_" + str( name ) + " " + str( yOffset ) + "\n")
        yOffset += nmpc[ name ].shape[ 0 ]
    fw.write("\n\n")
    
    #
    # Generate steady state for NMPC
    #

    # Reference parameters
    refP = {'r0': measCableLength, 'ddelta0': steadyStateSpeed}

    # Get the steady state
    steadyState, dSS = getSteadyState(nmpc.dae, conf, refP['ddelta0'], refP['r0'], verbose = False)

    # Write the steady state to the file
    names = {"x": nmpc.dae.xNames(), "u": nmpc.dae.uNames(), "z": nmpc.dae.zNames()}
    for k, v in names.items():
        fw.write("const double ss_" + k + "[ " + str( len( v ) ) + " ] = {")
        fw.write(", ".join([repr( steadyState[ name ] ) + " /*" + name + "*/" for name in v]))
        fw.write("};\n\n")

    #
    # Generate reference for the NMPC
    #
    _, code = generateReference(nmpc, conf, measCableLength, steadyStateSpeed)
    fw.write( code )
    
    fw.write("#endif // NMPC_CONFIGURATION\n")
    fw.close()
