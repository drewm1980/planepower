
def makeProtoBuf(protoname, fieldnames):
    ret = []
    ret.append('message '+protoname+' {')
    for k,name in enumerate(fieldnames):
        ret.append('  required double '+name+' = '+str(k+1)+';')
    ret.append('}\n\n')
    return '\n'.join(ret)

def makeStruct(vecname,fieldnames):
    ret = []
    ret.append('struct '+vecname)
    ret.append('{')
    for k,name in enumerate(fieldnames):
        ret.append('  carousel_float_t '+name+'; /* '+str(k)+' */')
    ret.append('};\n\n')

    return '\n'.join(ret)

def makeProtoConverter(vecname,protoname,fieldnames):
    ret = []
    prototype = 'void ProtobufBridge::from'+vecname+'(MheMpc::'+protoname+' *proto, const '+vecname+' *data)'
    ret.append(prototype)
    ret.append('{')
    for name in fieldnames:
        ret.append('  proto->set_'+name.lower()+'( data->'+name+' );')
    ret.append('}\n\n')
    return ('\n'.join(ret), prototype+';\n')
