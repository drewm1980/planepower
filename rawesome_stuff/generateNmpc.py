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
                'CXXFLAGS':'-O3 -fPIC -finline-functions',
                'CFLAGS':'-O3 -fPIC -finline-functions',
                'hideSymbols':True}
    exportpath =  nmpc.exportCode(NMPC.mpcOpts, NMPC.mpcIntOpts, cgOptions, {})

    for filename in ['acado_common.h','ocp.o']:
        fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)

    for filename in ['solver.hpp']:
        fullname = os.path.join(exportpath, 'qpoases', filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)

    structs = rawe.utils.mkprotobufs.writeStructs(nmpc.dae, 'NMPC', nmpc.yNames, nmpc.yNNames)
    f = open('nmpc_structs.h','w')
    f.write(structs)
    f.close()

    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
