import sys
import os
import shutil

import MHE

if __name__=='__main__':
    assert len(sys.argv) == 2, \
        'need to call generateMhe.py with the properties directory'
    propsDir = sys.argv[1]
    mhert = MHE.makeMhe(propertiesDir=propsDir)

    for filename in ['acado_common.h','ocp.so', os.path.join('qpoases','solver.hpp')]:
        fullname = os.path.join(mhert._exportpath, filename)
        assert os.path.isfile(fullname), fullname+' is not a file'
        shutil.copy(fullname, filename)
