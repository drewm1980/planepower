#!/usr/bin/env python

import sys
import os
import shutil

import rawe

import numpy as np

from rawe.models.arianne_conf import makeConf

#
# We import the MHE that is tested on real measurements, not the one used in simulations
#
from offline_mhe_test import MHE
from offline_mhe_test import carouselModel

from rawekite.carouselSteadyState import getSteadyState

if __name__=='__main__':
    assert len(sys.argv) == 2, \
        'need to call generateMhe.py with the properties directory'
    propsDir = sys.argv[1]

    mhe = MHE.makeMhe(MHE.samplingTime, propertiesDir = propsDir)

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

    for k, v in MHE.mheWeights.items():
        fw.write("#define weight_" + str( k ) + " " + repr( v ) + "\n")
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
    daeSim = carouselModel.makeModel(conf, propsDir)
    
    # Cable length which we are going to supply to the MHE as a fake measurement
    measCableLength = 2.0
    
    # Speed for the steady state calculation
    steadyStateSpeed = -4.0

    # Reference parameters
    refP = {'r0': measCableLength,
            'ddelta0': steadyStateSpeed,
            }

    # Get the steady state
    steadyState, dSS = getSteadyState(daeSim, conf, refP['ddelta0'], refP['r0'])
        
    xlen = len( daeSim.xNames() )
    fw.write("// " + str(daeSim.xNames()) + "\n");
    fw.write("const double ss_x[ " + str( xlen ) + " ] = {")
    for k, name in enumerate(daeSim.xNames()):
        fw.write(repr(steadyState[ name ]))
        if k < (xlen - 1):
            fw.write(", ") 
    fw.write("};\n\n")
    
    ulen = len( daeSim.uNames() )
    fw.write("// " + str(daeSim.uNames()) + "\n");
    fw.write("const double ss_u[ " + str( ulen ) + " ] = {")
    for k, name in enumerate(daeSim.uNames()):
        fw.write(repr(steadyState[ name ]))
        if k < (ulen - 1):
            fw.write(", ") 
    fw.write("};\n\n")
    
    zlen = len( daeSim.zNames() )
    fw.write("// " + str(daeSim.zNames()) + "\n");
    fw.write("const double ss_z[ " + str( zlen ) + " ] = {")
    for k, name in enumerate(daeSim.zNames()):
        fw.write(repr(steadyState[ name ]))
        if k < (zlen - 1):
            fw.write(", ") 
    fw.write("};\n\n")
    
    fw.write("#endif // MHE_CONFIGURATION\n")
    fw.close()
    
    #
    # Generate protobuf specs for the MHE
    #
    
    xNames = ""
    for k, name in enumerate( mhe.dae.xNames() ):
        xNames = xNames + "idx_" + str( name ) + " = " + str( k ) + "; "
        
    zNames = ""
    for k, name in enumerate( mhe.dae.zNames() ):
        zNames = zNames + "idx_" + str( name ) + " = " + str( k ) + "; "
        
    uNames = ""
    for k, name in enumerate( mhe.dae.uNames() ):
        uNames = uNames + "idx_" + str( name ) + " = " + str( k ) + "; "
    
    proto = """
package DynamicMheProto;

message DynamicMheMsg
{
    enum Configuration
    {
        N = %(N)d;
    }

    enum xNames
    {
        %(xNames)s
    }
    
    enum zNames
    {
        %(zNames)s
    }
    
    enum uNames
    {
        %(uNames)s
    }

    message Horizon
    {
        repeated float h = 1;
    }

    repeated Horizon x = 1;
    repeated Horizon z = 2;
    repeated Horizon u = 3;
    
    repeated Horizon y  = 4;
    repeated float   yN = 5;
    
    required int32 solver_status    = 6;
    required float kkt_value = 7;
    required float obj_value = 8;
    
    required float exec_fbd  = 9;
    required float exec_prep = 10;
    
    required double ts_trigger = 11;
    required double ts_elapsed = 12;
}
""" % {"N": MHE.mheHorizonN, "xNames": xNames, "zNames": zNames, "uNames": uNames}
    
    fw = open("DynamicMheTelemetry.proto", "w")
    fw.write( proto )
    fw.close()
