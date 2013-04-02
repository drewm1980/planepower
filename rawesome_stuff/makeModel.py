import rawe.models.carousel

import codegen_utils

if __name__=='__main__':
    print "creating model..."
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)

    w = dae['wind at altitude']
    dae['accel_magnitude'] = dae.ddt('dx')*dae.ddt('dx') + dae.ddt('dy')*dae.ddt('dy')
    
