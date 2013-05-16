# -*- coding: utf-8 -*-
"""
Created on Mon Apr 22 15:48:33 2013

@author: mzanon
"""

import rawe
#import casadi as C

from NMPC import makeNmpc
from MHE import makeMhe
from mpc_mhe_utils import *

from highwind_carousel_conf import conf

dae = rawe.models.carousel(conf)

# Simulation parameters
N_mpc = 10  # Number of MPC control intervals
N_mhe = 10  # Number of MHE control intervals
Ts = 0.1    # Sampling time
nSteps = 40 #Number of steps for the Rintegrator (also in MPC and MHE)
iType = 'INT_IRK_GL2' # Rintegrator type
Tf = 1.2    # Simulation duration

# Create the MPC class
mpcRT = makeNmpc(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)
mheRT = makeMhe(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)

# Create a simulation class
intOptions = {'type':'Idas', 'ts':Ts}
sim, simLog = InitializeSim(dae,intOptions)

# Generate a Rintegrator for linearizing the system
from rawe.dae import RtIntegrator
Rint = RtIntegrator(dae,ts=Ts, numIntegratorSteps=nSteps, integratorType=iType)

# Reference parameters
refP = {'r0':1.2,
        'ddelta0':2*np.pi,
        'z0':-0.1}
            
InitializeMPC(mpcRT,Rint,dae,conf,refP)
InitializeMHE(mheRT,Rint,dae,conf,refP)

#x2=Rint.step(x=mpcRT.x0,u=mpcRT.u[0,:])
#x1=sim.step(mpcRT.x0,mpcRT.u[0,:],{})
#print x1-mpcRT.x0
#print x2-mpcRT.x0
#print x2-x1
#
#assert(1==0)

new_out = sim.getOutputs(mpcRT.x[0,:],mpcRT.u[0,:],{})
new_y  = np.append(mheRT.x[-2,:],mheRT.u[-1,:])
simLog.log(new_x=mpcRT.x0,new_y=new_y,new_out=new_out)

# Simulation loop
time = 0
while time < Tf:
    
#    print (mheRT.x[:-1,:]-mheRT.y[:,:25])
#    print (mheRT.u-mheRT.y[:,25:])
#    print (mheRT.x[-1,:]-mheRT.yN)
    
#    np.savetxt('x.txt',mheRT.x)
#    np.savetxt('u.txt',mheRT.u)
#    np.savetxt('y.txt',mheRT.y)
#    np.savetxt('yN.txt',mheRT.yN)
#    np.savetxt('S.txt',mheRT.S)
#    np.savetxt('SN.txt',mheRT.SN)
    
    mheRT.preparationStep()
    fbret = mheRT.feedbackStep()
    if fbret != 0:
        raise Exception("MHE feedbackStep returned error code "+str(fbret))
    
    mpcRT.x0 = mheRT.x[-1,:]
#    mpcRT.x0 = mpcRT.x[0,:]
    
    mpcRT.preparationStep()
    fbret = mpcRT.feedbackStep()
    if fbret != 0:
        raise Exception("MPC feedbackStep returned error code "+str(fbret))
    SimulateAndShift(mpcRT,mheRT,sim,simLog,Rint,dae,conf,refP)
    
    time += Ts
    print time


plt.ion()

Fig_subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe','mpc'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)
Fig_subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)
Fig_subplot([['e11','e12','e13'],['e21','e22','e23'],['e31','e32','e33']],what=['sim','mhe'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)
Fig_subplot(['w1','w2','w3'],what=['sim','mhe'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)
Fig_subplot([['aileron','elevator'],['daileron','delevator']],what=['sim','mhe'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)
Fig_subplot([['cos_delta','sin_delta'],['ddelta'],['motor_torque']],what=['sim','mhe'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)
Fig_subplot([['r'],['dr'],['ddr']],what=['sim','mhe'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)
Fig_subplot([['c'],['cdot']],what=['sim','mhe'],simLog=simLog,mheLog=mheRT,mpcLog=mpcRT)


#mpcLog.plot(['x','v'],when='all')

#plt.figure()
#for k in range(1,N_mpc):
#    plt.plot(np.array(mpcLog._log['x'])[k,:,0],np.array(mpcLog._log['x'])[k,:,1])
#plt.grid()

plt.show()
