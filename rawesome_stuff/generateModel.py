import rawe
import casadi as C

if __name__=='__main__':
    print "creating model..."
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)
    
    modelFile = dae.acadoModelGen()

    filename = 'out/model.cpp'
    print 'exporting model to "'+filename+'"...'
    f = open(filename,'w')
    f.write(modelFile)
    f.close()
