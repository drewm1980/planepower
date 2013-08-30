import sys
import os
import shutil

import rawe

import numpy as np

#
# We import the MHE that is tested on real measurements, not the one used in simulations
#
from offline_mhe_test import MHE

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
''')
    for k, v in MHE.mheWeights.items():
        fw.write("#define weight_" + str( k ) + " " + str( v ) + "\n")
    fw.write("#endif // MHE_WEIGHTS\n")
    fw.close()