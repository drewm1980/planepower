import rawe
import sys

from highwind_carousel_conf import conf
import carouselModel

if __name__=='__main__':
    assert len(sys.argv) == 2, 'need to call generateProtoGlue.py with the properties directory'
    dae = carouselModel.makeModel(conf,propertiesDir=sys.argv[1])

    autogenDir = '.'
    topname = 'Carousel'
    mheHorizN = 10
    mpcHorizN = 10
    rawe.utils.mkprotobufs.writeAll(dae, topname, autogenDir, mheHorizN=10, mpcHorizN=10)
