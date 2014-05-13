#!/usr/bin/env python

import sys
import os
import shutil

from rawe.ocp.Ocp import generateProto

from rawe.models.arianne_conf import makeConf

#
# We import the MHE that is tested on real measurements, not the one used in simulations
#
from offline_mhe_test import MHE
from offline_mhe_test import carouselModel

from rawekite.carouselSteadyState import getSteadyState

if __name__=='__main__':
    assert len(sys.argv) == 2 or len(sys.argv) == 3, \
        'need to call generateMhe.py with the properties directory'
    propsDir = sys.argv[1]

    mhe = MHE.makeMhe(MHE.samplingTime, propertiesDir = propsDir)
    
    #
    # Generate protobuf specs for the MHE
    #
    fw = open("DynamicMheTelemetry.proto", "w")
    fw.write( generateProto(mhe, "DynamicMhe") )
    fw.close()
    
    if len(sys.argv) == 3 and sys.argv[ 2 ] == "proto_only":
        print "MHE protobuf is generated and I am out..."
        sys.exit( 0 )

    # Options for code compilation
    cgOptions = {
        'CXX': 'clang++', 'CC': 'clang',
        'CXXFLAGS': '-fPIC -O3 -march=native -mtune=native',
        'CFLAGS': '-fPIC -O3 -march=native -mtune=native',
        # For OROCOS compilation, this option is mandatory
        'hideSymbols': True
    }
    # Now export the code
    exportpath = mhe.exportCode(MHE.mheOpts, MHE.mheIntOpts, cgOptions, {})

    # Copy the library and the headers to output location
    for filename in ['acado_common.h', 'acado_qpoases_interface.hpp', 'ocp.o']:
        fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname + ' is not a file'
        shutil.copy(fullname, filename)
    
    # Generate info file 
    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
    
    # Generate MHE configuration file

    Ts = MHE.samplingTime
    nDelay = 2
    numMarkers = 12
    
    fw = open("mhe_configuration.h", "w")
    fw.write(
'''
#ifndef MHE_CONFIGURATION
#define MHE_CONFIGURATION

'''
            )

    fw.write("// This file was created from the file: " + os.path.realpath(__file__) + "\n\n")

    fw.write("#define mhe_sampling_time " + repr( Ts ) + "\n");
    fw.write("#define mhe_ndelay " + str( nDelay ) + "\n");
    fw.write("#define mhe_num_markers " + repr( numMarkers ) + "\n");
    fw.write("\n\n")

    # Write weights
    for k, v in MHE.mheWeights.items():
        fw.write("#define weight_" + str( k ) + " " + repr( v ) + "\n")
    fw.write("\n\n")
    
    # Output offsets for measurements
    fw.write("// Measurement offsets\n")
    yOffset = 0
    for name in mhe.yxNames:
        fw.write("#define offset_" + str( name ) + " " + str( yOffset ) + "\n")
        yOffset += mhe[ name ].shape[ 0 ]
    for name in mhe.yuNames:
        fw.write("#define offset_" + str( name ) + " " + str( yOffset ) + "\n")
        yOffset += mhe[ name ].shape[ 0 ]
    fw.write("\n\n")

    #
    # Write indices
    #
    fw.write("// Differential variables\n")
    for k, name in enumerate( mhe.dae.xNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    
    fw.write("\n\n")
    fw.write("// Control variables\n")
    for k, name in enumerate( mhe.dae.uNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    fw.write("\n\n")
    
    #
    # Generate steady state for MHE
    #
    
    # Get the plane configuration parameters
    conf = makeConf()
    conf[ 'stabilize_invariants' ] = False
    conf[ 'useVirtualTorques' ]    = True
    conf[ 'useVirtualForces' ]     = False 
    
    # Cable length for steady state calculation
    steadyStateCableLength = 1.7
    # Speed for the steady state calculation
    steadyStateSpeed = -4.0

    # Reference parameters
    refP = {'r0': steadyStateCableLength,
            'ddelta0': steadyStateSpeed,
            }

    # Get the steady state
    steadyState, dSS = getSteadyState(mhe.dae, conf, refP['ddelta0'], refP['r0'], verbose = False)
        
    xlen = len( mhe.dae.xNames() )
    fw.write("// " + str(mhe.dae.xNames()) + "\n");
    fw.write("const double ss_x[ " + str( xlen ) + " ] = {")
    for k, name in enumerate(mhe.dae.xNames()):
        fw.write(repr(steadyState[ name ]))
        if k < (xlen - 1):
            fw.write(", ") 
    fw.write("};\n\n")
    
    ulen = len( mhe.dae.uNames() )
    fw.write("// " + str(mhe.dae.uNames()) + "\n");
    fw.write("const double ss_u[ " + str( ulen ) + " ] = {")
    for k, name in enumerate(mhe.dae.uNames()):
        fw.write(repr(steadyState[ name ]))
        if k < (ulen - 1):
            fw.write(", ") 
    fw.write("};\n\n")
    
    zlen = len( mhe.dae.zNames() )
    fw.write("// " + str(mhe.dae.zNames()) + "\n");
    fw.write("const double ss_z[ " + str( zlen ) + " ] = {")
    for k, name in enumerate(mhe.dae.zNames()):
        fw.write(repr(steadyState[ name ]))
        if k < (zlen - 1):
            fw.write(", ") 
    fw.write("};\n\n")
    
    fw.write("#endif // MHE_CONFIGURATION\n")
    fw.close()
