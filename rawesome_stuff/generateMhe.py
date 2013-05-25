import sys
import os
import shutil

import rawe
import MHE

if __name__=='__main__':
    assert len(sys.argv) == 2, \
        'need to call generateMhe.py with the properties directory'
    propsDir = sys.argv[1]

    mhe = MHE.makeMhe(propertiesDir=propsDir)
    cgOptions= {'CXX':'clang++', 'CC':'clang','hideSymbols':True}
    exportpath =  mhe.exportCode(MHE.mheOpts, MHE.mheIntOpts, cgOptions, {})

    for filename in ['acado_common.h','ocp.o']:
        fullname = os.path.join(exportpath, filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)

    for filename in ['solver.hpp']:
        fullname = os.path.join(exportpath, 'qpoases', filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)

    structs = rawe.utils.mkprotobufs.writeStructs(mhe.dae, 'MHE', mhe.yNames, mhe.yNNames)
    f = open('mhe_structs.h','w')
    f.write(structs)
    f.close()

    f = open('whereami.txt','w')
    f.write(exportpath+'\n')
    f.close()
