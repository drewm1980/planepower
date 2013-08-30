import sys
import os
import shutil

import rawe
import NMPC

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
