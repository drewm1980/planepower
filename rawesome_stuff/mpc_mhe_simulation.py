# -*- coding: utf-8 -*-
"""
Created on Mon Apr 22 15:48:33 2013

@author: mzanon
"""

import rawe

from NMPC import makeNmpc
from MHE import makeMhe
from mpc_mhe_utils import *

from highwind_carousel_conf import conf
import carouselModel

dae = carouselModel.makeModel(conf)

# Simulation parameters
N_mpc = 10  # Number of MPC control intervals
N_mhe = 10  # Number of MHE control intervals
Ts = 0.1    # Sampling time
nSteps = 20 #Number of steps for the Rintegrator (also in MPC and MHE)
iType = 'INT_IRK_GL2' # Rintegrator type
iType = 'INT_IRK_RIIA3' # Rintegrator type
Tf = 1.   # Simulation duration

# Create the MPC class
mpcRT, intOpts = makeNmpc(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)
mheRT, _       = makeMhe(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)

# Create a simulation class
sim = InitializeSim(dae,'Idas',Ts,intOpts)
#sim, simLog = InitializeSim(dae,'RtIntegrator',Ts,intOpts)

# Generate a Rintegrator for linearizing the system
Rint = rawe.RtIntegrator(dae,ts=Ts, options=intOpts, measurements=dae['ConstDelta'])


# Reference parameters
refP = {'r0':1.2,
        'ddelta0':2*np.pi,
        'z0':-0.1}
            
InitializeMPC(mpcRT,Rint,dae,conf,refP)
InitializeMHE(mheRT,Rint,dae,conf,refP)

new_out = sim.getOutputs(mpcRT.x[0,:],mpcRT.u[0,:],{})
new_y  = np.append(mheRT.x[-2,:],mheRT.u[-1,:])
sim.log(new_x=mpcRT.x0,new_y=new_y,new_out=new_out)

# Simulation loop
time = 0
while time < Tf:
    
    mheRT.preparationStep()
    mheRT.feedbackStep()

#    mpcRT.x0 = mheRT.x[-1,:]
#    mpcRT.preparationStep()
#    mpcRT.feedbackStep()

    SimulateAndShift(mpcRT,mheRT,sim,Rint,dae,conf,refP)
    mheRT.shiftStatesControls()
    
    time += Ts
    print "sim time:",time


plt.ion()

plotter = Plotter(sim,mheRT,mpcRT)
mheRT.plot('daileron',when='all',style='o',offset=0)
mheRT.plot('daileron',when='all',offset=0)
plt.show()
plotter.subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe','mpc'])
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
plotter.subplot([['c'],['cdot'],['ConstDelta']],what=['sim','mhe'])

#mpcrt.plot(['x','v'],when='all')

plt.show()
