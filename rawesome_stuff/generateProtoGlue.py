import rawe
import sys

from highwind_carousel_conf import getConf
import carouselModel

import MHE
import NMPC

if __name__=='__main__':
    assert len(sys.argv) == 2, 'need to call generateProtoGlue.py with the properties directory'
    conf = getConf()
    dae = carouselModel.makeModel(conf,propertiesDir=sys.argv[1])

    autogenDir = '.'
    topname = 'Carousel'
    rawe.utils.mkprotobufs.writeAll(dae, topname, autogenDir,
                                    mheHorizN=MHE.mheHorizonN,
                                    mpcHorizN=NMPC.mpcHorizonN,
                                    measurements=MHE.measNames,
                                    measurementsEnd=MHE.endMeasNames)
