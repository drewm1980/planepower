import rawe
import sys

from rawe.models.arianne_conf import makeConf
import carouselModel

import MHE
import NMPC

if __name__=='__main__':
    assert len(sys.argv) == 2, 'need to call generateProtoGlue.py with the properties directory'
    conf = makeConf()
    dae = carouselModel.makeModel(conf,propertiesDir=sys.argv[1])

    autogenDir = '.'
    topname = 'Carousel'
    rawe.utils.mkprotobufs.writeAll(dae, topname, autogenDir,
                                    mheHorizN=MHE.mheHorizonN,
                                    mpcHorizN=NMPC.mpcHorizonN,
                                    measurementsX=MHE.measX,
                                    measurementsU=MHE.measU)
