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
dae['ConstR1'] = dae['e11']*dae['e11'] + dae['e12']*dae['e12'] + dae['e13']*dae['e13'] - 1
dae['ConstR2'] = dae['e11']*dae['e21'] + dae['e12']*dae['e22'] + dae['e13']*dae['e23']
dae['ConstR3'] = dae['e11']*dae['e31'] + dae['e12']*dae['e32'] + dae['e13']*dae['e33']
dae['ConstR4'] = dae['e21']*dae['e21'] + dae['e22']*dae['e22'] + dae['e23']*dae['e23'] - 1
dae['ConstR5'] = dae['e21']*dae['e31'] + dae['e22']*dae['e32'] + dae['e23']*dae['e33']
dae['ConstR6'] = dae['e31']*dae['e31'] + dae['e32']*dae['e32'] + dae['e33']*dae['e33'] - 1
dae['Const'] = dae['x']*dae['x']/2 + dae['y']*dae['y']/2 + dae['z']*dae['z']/2 - \
               dae['r']*dae['r']/2
dae['dConst'] = dae['dx']*dae['x'] + dae['dy']*dae['y'] + dae['dz']*dae['z']
dae['ConstDelta'] = dae['cos_delta']**2 + dae['sin_delta']**2 - 1

# Simulation parameters
N_mpc = 10  # Number of MPC control intervals
N_mhe = 10  # Number of MHE control intervals
Ts = 0.1    # Sampling time
nSteps = 20 #Number of steps for the Rintegrator (also in MPC and MHE)
iType = 'INT_IRK_GL2' # Rintegrator type
Tf = 1.2    # Simulation duration

# Create the MPC class
mpcRT = makeNmpc(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)
mheRT = makeMhe(dae,N=N_mpc,dt=Ts,nSteps=nSteps,iType=iType)

# Create a simulation class
intOptions = {'type':'Idas', 'ts':Ts}
sim, simLog = InitializeSim(dae,intOptions)

# Generate a Rintegrator for linearizing the system
#from rawe.dae import rawe.RtIntegrator
Rint = rawe.RtIntegrator(dae,ts=Ts, numIntegratorSteps=nSteps, integratorType=iType)

# Reference parameters
refP = {'r0':1.2,
        'ddelta0':2*np.pi,
        'z0':-0.1}
            
InitializeMPC(mpcRT,Rint,dae,conf,refP)
InitializeMHE(mheRT,Rint,dae,conf,refP)

#np.savetxt('x.txt',mheRT.x)
#np.savetxt('u.txt',mheRT.u)
#np.savetxt('y.txt',mheRT.y)
#np.savetxt('yN.txt',mheRT.yN)
#np.savetxt('S.txt',mheRT.S)
#np.savetxt('SN.txt',mheRT.SN)

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
    
    mheRT.preparationStep()
    mheRT.feedbackStep()
    
    mpcRT.x0 = mheRT.x[-1,:]
#    mpcRT.x0 = mpcRT.x[0,:]
    
    mpcRT.preparationStep()
    mpcRT.feedbackStep()

    SimulateAndShift(mpcRT,mheRT,sim,simLog,Rint,dae,conf,refP)
    
    time += Ts
    print time


plt.ion()

plotter = Plotter(simLog,mheRT,mpcRT)
plotter.subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe','mpc'])
plotter.subplot([['x','y','z'],['dx','dy','dz']],what=['sim','mhe'])
plotter.subplot([['e11','e12','e13'],['e21','e22','e23'],['e31','e32','e33']],what=['sim','mhe'])
plotter.subplot(['w1','w2','w3'],what=['sim','mhe'])
plotter.subplot([['aileron','elevator'],['daileron','delevator']],what=['sim','mhe'])
plotter.subplot([['cos_delta','sin_delta'],['ddelta'],['motor_torque']],what=['sim','mhe'])
plotter.subplot([['r'],['dr'],['ddr']],what=['sim','mhe'])
plotter.subplot([['c'],['cdot']],what=['sim','mhe'])
plotter.subplot([['kkt'],['objective'],['prep_time','fb_time']],what=['mpc','mhe'])
plotter.plot(['kkt'],what=['mpc','mhe'])
plotter.plot(['ConstR1','ConstR2','ConstR3','ConstR4','ConstR5','ConstR6'],what=['sim','mhe'])
plotter.subplot([['Const'],['dConst'],['ConstDelta']],what=['sim','mhe'])

#mpcLog.plot(['x','v'],when='all')

#plt.figure()
#for k in range(1,N_mpc):
#    plt.plot(np.array(mpcLog._log['x'])[k,:,0],np.array(mpcLog._log['x'])[k,:,1])
#plt.grid()

plt.show()
