import rawe.models.carousel

import codegen_utils

if __name__=='__main__':
    print "creating model..."
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)

    # convert model to acado format
    modelFile = dae.acadoModelGen()
#    filename = 'out/model.cpp'
#    print 'exporting model to "'+filename+'"...'
#    f = open(filename,'w')
#    f.write(modelFile)
#    f.close()

    from dummy_model import dummyModel
    dae = dummyModel()
    # write a bunch of glue to convert from vectors to structs to protobufs
    structs = ''
    protoConverters = ''
    protoConverterPrototypes = ''
    protobufs = ''
    for (vecname,protoname,fieldnames) in \
            [('DiffStateVec','DifferentialStates',dae.xNames()),
             ('AlgVarVec','AlgebraicVars',dae.zNames()),
             ('ControlVec','Controls',dae.uNames()),
             ('ParamVec','Parameters',dae.pNames())]:
        protobufs += codegen_utils.makeProtoBuf(protoname, fieldnames)
        structs += codegen_utils.makeStruct(vecname,fieldnames)
        (pc,pcpt) = codegen_utils.makeProtoConverter(vecname,protoname,fieldnames)
        protoConverters += pc
        protoConverterPrototypes += pcpt
    print '------------ PROTOBUFS: -----------'
    print protobufs
    print '------------ STRUCTS: -----------'
    print structs
    print '------------ PROTO CONVERTERS: -----------'
    print protoConverters
    print '------------ PROTO CONVERTER PROTOTYPES: -----------'
    print protoConverterPrototypes

