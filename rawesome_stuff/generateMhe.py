import sys
import os
import shutil

import rawe
import MHE

if __name__=='__main__':
    assert len(sys.argv) == 2, \
        'need to call generateMhe.py with the properties directory'
    propsDir = sys.argv[1]

    mhe = MHE.makeMhe(propertiesDir = propsDir)

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
    
    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
