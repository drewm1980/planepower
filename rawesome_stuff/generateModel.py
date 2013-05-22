import rawe.models.carousel

if __name__=='__main__':
    from highwind_carousel_conf import conf
    import carouselModel
    dae = carouselModel.makeModel(conf)
    autogenDir = 'autogen'
    topname = 'Carousel'
    mheHorizN = 10
    mpcHorizN = 10
    rawe.utils.mkprotobufs.writeAll(dae, topname, autogenDir,
                                    haskellDirs=['plot-ho-matic'], mheHorizN=10, mpcHorizN=10)
