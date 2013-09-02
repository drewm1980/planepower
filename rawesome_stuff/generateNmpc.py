import sys
import os
import shutil

import rawe
import NMPC

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
        'need to call generateNmpc.py with the properties directory'
    propsDir = sys.argv[1]

    nmpc = NMPC.makeNmpc(propertiesDir=propsDir)
    cgOptions= {'CXX':'clang++', 'CC':'clang',
                'CXXFLAGS':'-O3 -fPIC -finline-functions -march=native',
                'CFLAGS':'-O3 -fPIC -finline-functions -march=native',
                'hideSymbols':True}
    exportpath =  nmpc.exportCode(NMPC.mpcOpts, NMPC.mpcIntOpts, cgOptions, {})

    # Copy the library and the headers to output location
    for filename in ['acado_common.h', 'solver.hpp', 'ocp.o']:
        if filename == 'solver.hpp':
            fullname = os.path.join(exportpath, 'qpoases/' + filename)
        else:
            fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)
    
    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
    
    Ts = 0.02
    
        # Generate a data file with weights
    fw = open("mpc_configuration.h", "w")
    fw.write(
'''
#ifndef NMPC_CONFIGURATION
#define NMPC_CONFIGURATION

'''
            )
    
    fw.write("#define mpc_sampling_time " + repr( Ts ) + "\n");
    fw.write("\n\n")
    
    fw.write("// " + str(nmpc.yxNames + nmpc.yuNames) + "\n");
    fw.write("const double mpc_weights[ " + str(len(nmpc.yxNames + nmpc.yuNames)) + " ] = {")
    yLen = len( nmpc.yxNames + nmpc.yuNames )
    for k, name in enumerate( nmpc.yxNames + nmpc.yuNames ):
        fw.write(repr(NMPC.mpcWeights[ name ]))
        if k < (yLen - 1):
            fw.write(", ")
    fw.write("};\n\n")
    
    fw.write("// Differential variables\n")
    for k, name in enumerate( nmpc.dae.xNames() ):
        fw.write("#define idx_" + str( name ) + " " + str( k ) + "\n")
    
    fw.write("\n\n")
    fw.write("// Control variables\n")
    for k, name in enumerate( nmpc.dae.uNames() ):
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
    
    fw.write("#endif // NMPC_CONFIGURATION\n")
    fw.close()
    
