import rawe
import casadi as C

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

    # write a bunch of glue to convert from vectors to structs to protobufs
    structs = ''
    protoConverters = ''
    protoConverterPrototypes = ''
    protobufs = ''
    for (vecname,fieldnames) in [('DiffStateVec',dae.xNames()),
                                 ('AlgVarVec',dae.zNames()),
                                 ('ControlVec',dae.uNames()),
                                 ('ParamVec',dae.pNames())]:
        protobufs += codegen_utils.makeProtoBuf(vecname, fieldnames)
        structs += codegen_utils.makeStruct(vecname,fieldnames)
        (pc,pcpt) = codegen_utils.makeProtoConverter(vecname,fieldnames)
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

