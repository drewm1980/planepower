# -*- coding: utf-8 -*-
"""
Created on Mon Apr 22 15:48:33 2013

@author: mzanon
"""

import rawe

import NMPC
import MHE
from mpc_mhe_utils import *
from bufStuff.protobufBridgeWrapper import ProtobufBridge

from highwind_carousel_conf import conf
import carouselModel

from common_conf import Ts

conf['stabilize_invariants'] = False
dae = carouselModel.makeModel(conf)
conf['stabilize_invariants'] = True
daeSim = carouselModel.makeModel(conf)

Covariance = {'marker_positions':1e3,
              'IMU_angular_velocity':1.,
              'IMU_acceleration':10.,
              'r':1.,
              'cos_delta':1., 'sin_delta':1.,
              'aileron':1e-2, 'elevator':1e-2,
              'daileron':1e-4, 'delevator':1e-4,
              'motor_torque':20.,
              'ddr':1e-4,
              'dmotor_torque':1.,
              'dddr':1e-4}

for name in dae.xNames() + dae.uNames():
    Covariance[name] = 1.

# Define the weights
Wp  = 1e-1
Wdp = 1e-1
We  = 1.
Ww  = 1.
Wr  = 1.
Wdr  = Wr
Wdelta = 1e2
Wddelta = Wdelta
Wae = 1e2
Wddr = 1.
Wmt = 1e-1

Wdmt = 1.
Wdddr = 1.
Wdae = 1.

MPCweights = {}
for name in ['x','y','z']: MPCweights[name] = Wp
for name in ['dx','dy','dz']: MPCweights[name] = Wdp
for name in ['e11', 'e12', 'e13', 'e21', 'e22', 'e23', 'e31', 'e32', 'e33']: MPCweights[name] = We
for name in ['w1','w2','w3']: MPCweights[name] = Ww
MPCweights['r'] = Wr
MPCweights['dr'] = Wdr
MPCweights['ddr'] = Wddr
MPCweights['cos_delta'] = MPCweights['sin_delta'] = Wdelta
MPCweights['ddelta'] = Wddelta
MPCweights['motor_torque'] = Wmt
MPCweights['aileron'] = MPCweights['elevator'] = Wae

MPCweights['dmotor_torque'] = Wdmt
MPCweights['dddr'] = Wdddr
MPCweights['daileron'] = MPCweights['delevator'] = Wdae

## Simulation parameters
Tf = 20.0   # Simulation duration

# Create the MPC class
mpcRT = NMPC.makeNmpc(dae,lqrDae=daeSim)
mheRT = MHE.makeMhe(dae)

# Create a simulation class
sim = InitializeSim(daeSim,'Idas',Ts,NMPC.mpcIntOpts)
#sim, simLog = InitializeSim(dae,'RtIntegrator',Ts,intOpts)

# Generate a Rintegrator for linearizing the system
Rint = rawe.RtIntegrator(dae,ts=Ts, options=mpcRT._integratorOptions, measurements=dae['ConstDelta'])


# Reference parameters
refP = {'r0':1.2,
        'ddelta0':2*np.pi,
        'z0':-0.1}
            
InitializeMPC(mpcRT,Rint,dae,conf,refP,MPCweights)
InitializeMHE(mheRT,Rint,dae,conf,refP,Covariance)

#np.savetxt('A.txt',mpcRT._integratorLQR.dx1_dx0)
#np.savetxt('B.txt',mpcRT._integratorLQR.dx1_du)
#np.savetxt('Q.txt',mpcRT.Q)
#np.savetxt('R.txt',mpcRT.R)

new_out = sim.getOutputs(mpcRT.x[0,:],mpcRT.u[0,:],{})
new_y  = np.append(mheRT.x[-2,:],mheRT.u[-1,:])
sim.log(new_x=mpcRT.x0,new_y=new_y,new_out=new_out)

# Simulation loop
time = 0

pbb = ProtobufBridge()
while time < Tf:
    mheRT.preparationStep()
    mheRT.feedbackStep()

    mpcRT.x0 = mheRT.x[-1,:]
    
    mpcRT.preparationStep()
    mpcRT.feedbackStep()

    pbb.setMhe(mheRT)
    pbb.setMpc(mpcRT)
    pbb.setSimState(sim._log['x'][-1], mpcRT.u[0,:])
    pbb.sendMessage()

    SimulateAndShift(mpcRT,mheRT,sim,Rint,dae,conf,refP)
    
    time += Ts
    print "sim time:",time


plt.ion()

plotter = Plotter(sim,mheRT,mpcRT)

plotter.subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe'])
plotter.subplot([['e11','e12','e13'],['e21','e22','e23'],['e31','e32','e33']],what=['sim','mhe'])
plotter.subplot(['w1','w2','w3'],what=['sim','mhe'])
plotter.subplot([['aileron','elevator'],['daileron','delevator']],what=['sim','mhe'])
plotter.subplot([['cos_delta','sin_delta'],['ddelta'],['motor_torque']],what=['sim','mhe'])
plotter.subplot([['r'],['dr'],['ddr']],what=['sim','mhe'])
plotter.subplot([['c'],['cdot']],what=['sim','mhe'])
plotter.subplot([['_kkt'],['_objective'],['_prep_time','_fb_time']],what=['mpc','mhe'])
plotter.plot(['_kkt'],what=['mpc','mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim','mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim'])
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim','mhe'])
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim'])


#mpcrt.plot(['x','v'],when='all')

plt.show()
