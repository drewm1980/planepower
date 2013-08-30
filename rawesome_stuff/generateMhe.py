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
        'CXXFLAGS': '-O3 -fPIC -finline-functions -march=native -DACADO_CMAKE_BUILD',
        'CFLAGS': '-O3 -fPIC -finline-functions -march=native -DACADO_CMAKE_BUILD',
        # For OROCOS compilation, this option is mandatory
        'hideSymbols': True
    }
    # Now export the code
    exportpath = mhe.exportCode(MHE.mheOpts, MHE.mheIntOpts, cgOptions, {})

    # Copy the library and the headers to the 
    for filename in ['acado_common.h', 'solver.hpp', 'ocp.o']:
        if filename == 'solver.hpp':
            fullname = os.path.join(exportpath, 'qpoases/' + filename)
        else:
            fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)
    
    # Generate info file 
    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
    
    # Generate a data file with weights
    fw = open("mhe_weights.h", "w")
    fw.write(
'''
#ifndef MHE_WEIGHTS
#define MHE_WEIGHTS
'''
            )
    for k, v in MHE.mheWeights.items():
        fw.write("#define weight_" + str( k ) + " " + repr( v ) + "\n")
    fw.write("#endif // MHE_WEIGHTS\n")
    fw.close()
    
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
    
    fss = open("mhe_steady_state.h", "w")
    fss.write(
'''
#ifndef MHE_STEADY_STATE
#define MHE_STEADY_STATE
'''
            )
    
    xlen = len( daeSim.xNames() )
    fss.write("// " + str(daeSim.xNames()) + "\n");
    fss.write("const double ss_x[ " + str( xlen ) + " ] = {")
    for k, name in enumerate(daeSim.xNames()):
        fss.write(repr(steadyState[ name ]))
        if k < (xlen - 1):
            fss.write(", ") 
    fss.write("};\n\n")
    
    ulen = len( daeSim.uNames() )
    fss.write("// " + str(daeSim.uNames()) + "\n");
    fss.write("const double ss_u[ " + str( ulen ) + " ] = {")
    for k, name in enumerate(daeSim.uNames()):
        fss.write(repr(steadyState[ name ]))
        if k < (ulen - 1):
            fss.write(", ") 
    fss.write("};\n\n")
    
    zlen = len( daeSim.zNames() )
    fss.write("// " + str(daeSim.zNames()) + "\n");
    fss.write("const double ss_z[ " + str( zlen ) + " ] = {")
    for k, name in enumerate(daeSim.zNames()):
        fss.write(repr(steadyState[ name ]))
        if k < (zlen - 1):
            fss.write(", ") 
    fss.write("};\n\n")
    fss.write("#endif // MHE_STEADY_STATE\n")
    fss.close()
    