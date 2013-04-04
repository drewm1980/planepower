import rawe
import rawe.models.carousel
import casadi as C
from camModel import*

import codegen_utils
def cross(a,b):
    c = C.vertcat([a[1]*b[2]-a[2]*b[1],
                   a[2]*b[0]-a[0]*b[2],
                   a[0]*b[1]-a[1]*b[0]])
    return c

if __name__=='__main__':
    print "creating model..."
    from highwind_carousel_conf import conf
    dae = rawe.models.carousel(conf)

    dae['accel_magnitude'] = dae.ddt('dx')*dae.ddt('dx') + dae.ddt('dy')*dae.ddt('dy')
    #ddp = C.vertcat([dae['ddx'],dae['ddy'],dae['ddz']])    
    #dw = C.vertcat([dae['dw1'],dae['dw2'],dae['dw3']])    
    ddp = C.vertcat([C.ssym('ddx'),C.ssym('ddy'),C.ssym('ddz')])
    dw = C.vertcat([C.ssym('dw1'),C.ssym('dw2'),C.ssym('dw3')])
    x =   dae['x']
    y =   dae['y']
    z =   dae['z']

    dx  =  dae['dx']
    dy  =  dae['dy']
    dz  =  dae['dz']

    w1  =  dae['w1']
    w2  =  dae['w2']
    w3  =  dae['w3']
    w = C.vertcat([w1,w2,w3])
    ddelta = dae['ddelta']
    dddelta = C.ssym('dddelta')
    r = dae['r']
    dr = dae['dr']
    e11 = dae['e11']
    e12 = dae['e12']
    e13 = dae['e13']

    e21 = dae['e21']
    e22 = dae['e22']
    e23 = dae['e23']

    e31 = dae['e31']
    e32 = dae['e32']
    e33 = dae['e33']
    R = C.veccat( [dae[n] for n in ['e11', 'e12', 'e13',
                                        'e21', 'e22', 'e23',
                                        'e31', 'e32', 'e33']]
                      ).reshape((3,3))

    rA = conf['rArm']
    g = conf['g']
    #RIMU = conf['RIMU']
    pIMU = C.ssym("pIMU",3,1)
    RIMU = C.ssym("RIMU",3,3)
    ddpIMU = C.mul(R.T,ddp) - ddelta**2*C.mul(R.T,C.vertcat([x+rA,y,0])) + 2*ddelta*C.mul(R.T,C.vertcat([-dy,dx,0])) + dddelta*C.mul(R.T,C.vertcat([-y,x+rA,0])) + C.mul(R.T,C.vertcat([0,0,g]))
    aShift = cross(dw,pIMU)
    dae['acceleration IMU'] = C.mul(RIMU,ddpIMU+aShift)
    dae['angular velocities IMU'] = C.mul(RIMU,C.vertcat([w1,w2,w3]))
    
    dae['marker positions'] = fullCamModel(dea,conf)
    
    
    
