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
Tf = 10.    # Simulation duration

# Create the MPC class
mpcRT = makeNmpc(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)
mheRT = makeMhe(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)

# Create a simulation class
intOptions = {'type':'Idas', 'ts':Ts}
sim, simLog = InitializeSim(dae,intOptions)

# Generate a Rintegrator for linearizing the system
from rawe.dae.rienIntegrator import RienIntegrator
Rint = RienIntegrator(dae,ts=Ts, numIntegratorSteps=nSteps, integratorType=iType)

# Reference parameters
refP = {'r0':1.2,
        'ddelta0':2*np.pi,
        'z0':-0.1}
            
mpcLog = InitializeMPC(mpcRT,Rint,dae,conf,refP)
mheLog = InitializeMHE(mheRT,Rint,dae,conf,refP)

#x2=Rint.step(x=mpcRT.x0,u=mpcRT.u[0,:])
#x1=sim.step(mpcRT.x0,mpcRT.u[0,:],{})
#print x1-mpcRT.x0
#print x2-mpcRT.x0
#print x2-x1
#
#assert(1==0)

#outs = sim.getOutputs(mpcRT.x[0,:],mpcRT.u[0,:],{})
new_y  = np.append(mheRT.x[-2,:],mheRT.u[-1,:])
simLog.log(mpcRT.x0,new_y,[])

time = 0
while time < Tf:
    
    mheRT.preparationStep()
    fbret = mheRT.feedbackStep()
    if fbret != 0:
        raise Exception("MHE feedbackStep returned error code "+str(fbret))
    
    mheLog.log(mheRT)
    
    mpcRT.preparationStep()
    fbret = mpcRT.feedbackStep()
    if fbret != 0:
        raise Exception("MPC feedbackStep returned error code "+str(fbret))
    
    mpcLog.log(mpcRT)
    
    # Get the measurement BEFORE simulating
#    outs = sim.getOutputs(mpcRT.x[0,:],mpcRT.u[0,:],{})
#    new_y  = np.squeeze(outs['measurements'])
    new_y = np.append(mpcRT.x[0,:],mpcRT.u[0,:])
    # Simulate the system
    new_x = sim.step(mpcRT.x[0,:],mpcRT.u[0,:],{})  
    # Get the last measurement AFTER simulating
#    outs = sim.getOutputs(new_x,mpcRT.u[0,:],{})
#    new_yN = np.array([outs['measurementsN']])
    new_yN = np.squeeze(new_x)
    
    simLog.log(new_x,new_y,new_yN)
    
    # Assign the new initial value and shift
    mpcRT.x0 = np.squeeze(new_x)
    mpcRT.shift()
    mheRT.shift(new_y=new_y,new_yN=new_yN)
    
    time += Ts



plt.ion()

Fig_plot(['x','y','z'],what=['sim','mhe','mpc'],simLog=simLog,mheLog=mheLog,mpcLog=mpcLog)
Fig_plot(['x','y','z'],what=['sim','mhe'],simLog=simLog,mheLog=mheLog,mpcLog=mpcLog)

#mpcLog.plot(['x','v'])

#mpcLog.plot(['x','v'],when='all')

#plt.figure()
#for k in range(1,N_mpc):
#    plt.plot(np.array(mpcLog._log['x'])[k,:,0],np.array(mpcLog._log['x'])[k,:,1])
#plt.grid()

plt.show()