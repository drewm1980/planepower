# -*- coding: utf-8 -*-
"""
Created on Wed Apr 24 15:19:57 2013

@author: mzanon
"""
import numpy as np
import scipy
import copy
import matplotlib.pyplot as plt

import casadi as C
import rawe

from rawekite.carouselSteadyState import getSteadyState


def dlqr(A, B, Q, R, N=None):
    
    if N == None:
        N = np.zeros((Q.shape[0],R.shape[0]))
    
    P = scipy.linalg.solve_discrete_are(A, B, Q, R)
    
    k1 = np.dot(np.dot(B.T, P), B) + R
    k2 = np.dot(np.dot(B.T, P), A)
    K = np.linalg.solve(k1,k2)
    
    return K, P
    
def ComputeTerminalCost(integrator, xlin, ulin, Q, R, N=None): 
    
    integrator.x = xlin
    integrator.u = ulin
    integrator.step()
    A = integrator.dx1_dx0
    B = integrator.dx1_du
#    integrator.getOutputs()
    
    K, P = dlqr(A, B, Q, R, N=None)
    
    return K, P, A, B
    
def GenerateReference(dae,conf,refP):
    
    z0 = refP['z0']
    r0 = refP['r0']
    ddelta0 = refP['ddelta0']
    steadyState, _ = getSteadyState(dae,conf,ddelta0,r0,z0)
    
    xref = {}
    uref = {}
    for name in dae.xNames():
        xref[name] = steadyState[name]
    for name in dae.uNames():
        uref[name] = steadyState[name]
    
    return xref, uref

def InitializeMPC(mpcrt,integrator,dae,conf,refP):
        
    xref, uref = GenerateReference(dae,conf,refP)
    print xref
    N = mpcrt.u.shape[0]
    Xref = [xref[name] for name in dae.xNames()]
    Uref = [uref[name] for name in dae.uNames()]
    Xref = np.array( [Xref]*(N+1) )
    Uref = np.array( [Uref]*N     )
    
    ts = mpcrt._ts
    # The only part that changes is cos(delta), sin(delta) 
    CS = np.array([ [np.cos(k*ts*refP['ddelta0']), np.sin(k*ts*refP['ddelta0'])] for k in range(N+1) ])
    for k,name in enumerate(dae.xNames()):
        if name == 'cos_delta': Xref[:,k] = CS[:,0]
        if name == 'sin_delta': Xref[:,k] = CS[:,1]
    # Set the initial guess
    mpcrt.x = Xref
    mpcrt.u = Uref
    
    # Set the reference
    mpcrt.y = np.append(Xref[:-1,:],Uref,axis=1)
    mpcrt.yN = Xref[-1,:]
    
    # Set the initial state
    mpcrt.x0 = Xref[0,:]
    
    # Define the weights
    Wp  = 25.
    Wdp = 25.
    We  = 10.
    Ww  = 0.1
    Wr  = 100.
    Wdelta = 100.
    Wae = 250.
    
    Q = [Wp]*3 + [Wdp]*3 + [We]*9 + [Ww]*3 + [Wr]*2 + [Wdelta]*3 + [Wae]*2
    R = [1.0*1000000]*4
       
    mpcrt.S = np.diag( Q + R )*1e-2
    Q = np.diag( Q )*1e-2
    R = np.diag( R )*1e-2
    
#    K, P, A, B = ComputeTerminalCost(integrator, xref, uref, Q, R)
    P = np.eye(Q.shape[0])*10
    mpcrt.SN = P
    
    mpcLog = rawe.ocp.ocprt.Logger(mpcrt,dae)
    
    return mpcLog
   
def InitializeMHE(mhert,integrator,dae,conf,refP):
    
    xref, uref = GenerateReference(dae,conf,refP)
    
    N = mhert.u.shape[0]
    Xref = [xref[name] for name in dae.xNames()]
    Uref = [uref[name] for name in dae.uNames()]
    Xref = np.array( [Xref]*(N+1) )
    Uref = np.array( [Uref]*N     )
    
    ts = mhert._ts
    # The only part that changes is cos(delta), sin(delta) 
    CS = np.array([ [np.cos(k*ts*refP['ddelta0']), np.sin(k*ts*refP['ddelta0'])] for k in range(-N,1) ])
    for k,name in enumerate(dae.xNames()):
        if name == 'cos_delta': Xref[:,k] = CS[:,0]
        if name == 'sin_delta': Xref[:,k] = CS[:,1]
    # Set the initial guess
    mhert.x = Xref
    mhert.u = Uref
    
    # Set the measurements (temporary)
    mhert.y = np.append(Xref[:-1,:],Uref,axis=1)
    mhert.yN = Xref[-1,:]
    
    # Set the covariance (temporary)
    mhert.S  = np.eye(mhert.y.shape[1])
    mhert.S[25:,25:] = np.eye(4)*1
#    mhert.SN = np.eye(mhert.yN.shape[0])
    
    mheLog = rawe.ocp.ocprt.Logger(mhert,dae)
    
    return mheLog
    
def SimulateAndShift(mpcRT,mheRT,sim,mpcLog,mheLog,simLog):

    mheLog.log(mheRT)    
    mpcLog.log(mpcRT)
    
    # Get the measurement BEFORE simulating
#    outs = sim.getOutputs(mpcRT.x[0,:],mpcRT.u[0,:],{})
#    new_y  = np.squeeze(outs['measurements'])
    new_y = np.append(mpcRT.x[0,:],mpcRT.u[0,:]) + np.random.randn(29)*0.001
    # Simulate the system
    new_x = sim.step(mpcRT.x[0,:],mpcRT.u[0,:],{})
    # Get the last measurement AFTER simulating
    new_out = sim.getOutputs(new_x,mpcRT.u[0,:],{})
#    new_yN = np.array([outs['measurementsN']])
    new_yN = np.squeeze(new_x) + np.random.randn(25)*0.001
    
    simLog.log(new_x=new_x,new_y=new_y,new_yN=new_yN,new_out=new_out)
    
    # shift
    mpcRT.shift()
    mheRT.shift(new_y=new_y,new_yN=new_yN)
    
    
class SimLog(object):
    def __init__(self,dae,sim):
        self.xNames = dae.xNames()
        self.outputNames = dae.outputNames()
#        self.uNames = dae.uNames()
        self.Ts = sim._ts
        l=[]
        for n in self.outputNames: l.append([])
        self._log = {'x':[],'y':[],'yN':[],'outputs':dict(zip(self.outputNames,l))}
        
#        self.log()
    
    def log(self,new_x=None,new_y=None,new_yN=None,new_out=None):
        if new_x != None:
            self._log['x'].append(np.array(new_x))
        if new_y != None:
            self._log['y'].append(np.array(new_y))
        if new_yN != None:
            self._log['yN'].append(np.array(new_yN))
        if new_out != None:
            for name in new_out.keys():
                self._log['outputs'][name].append(np.array(new_out[name]))
        
    def _plot(self,names,title,style,when=0,showLegend=True):
        if isinstance(names,str):
            names = [names]
        assert isinstance(names,list)

        legend = []
        for name in names:
            assert isinstance(name,str)
            legend.append(name)

            # if it's a differential state
            if name in self.xNames:
                index = self.xNames.index(name)
                ys = np.squeeze(self._log['x'])[:,index]
                ts = np.arange(len(ys))*self.Ts
                plt.plot(ts,ys,style)
                
            if name in self.outputNames:
                index = self.outputNames.index(name)
                ys = np.squeeze(self._log['outputs'][name])
                ts = np.arange(len(ys))*self.Ts
                plt.plot(ts,ys,style)

        if title is not None:
            assert isinstance(title,str), "title must be a string"
            plt.title(title)
        plt.xlabel('time [s]')
        if showLegend is True:
            plt.legend(legend)
        plt.grid()
    
def InitializeSim(dae,intOptions):
    
    Ts = intOptions['ts']
    
    if intOptions['type'] == 'Idas':
        sim = rawe.sim.Sim(dae,Ts)
    elif intOptions['type'] == 'Rintegrator':
        from rawe.dae.rienIntegrator import RienIntegrator
        nSteps = intOptions['numIntegratorSteps']
        Type = intOptions['integratorType']
        sim = RienIntegrator(dae,ts=Ts, numIntegratorSteps=nSteps, integratorType=Type)
    else:
        raise Exception('integrator not supported')
    
    simLog = SimLog(dae,sim)
    
    return sim, simLog
    
def Fig_plot(names,title=None,style='',when=0,showLegend=True,what=[],mpcLog=None,mheLog=None,simLog=None):
    assert isinstance(what,list)
    
    fig = plt.figure()
    
    if title is None:
        if isinstance(names,str):
            title = names
        else:
            assert isinstance(names,list)
            if len(names) == 1:
                title = names[0]
            else:
                title = str(names)
    fig.canvas.set_window_title(str(title))

    plt.clf()
    
    if 'mpc' in what:
        if mpcLog == None: raise Exception('you must provide a mpc log to plot its variables')
        mpcLog._plot(names,None,'k',when='all',showLegend=True)
    if 'sim' in what:
        if simLog == None: raise Exception('you must provide a sim log to plot its variables')
        simLog._plot(names,None,'',when=0,showLegend=True)
    if 'mhe' in what:
        if mheLog == None: raise Exception('you must provide a mhe log to plot its variables')
        N = mheLog._log['x'][0].shape[0]
        if not isinstance(names,list):
            names = [names]
        if names[0] in mheLog.xNames:
            mheLog._plot(names,None,'o',when=N-1,showLegend=True)
        elif names[0] in mheLog.uNames:
            mheLog._plot(names,None,'o',when=N-2,showLegend=True)
        
def Fig_subplot(names,title=None,style='',when=0,showLegend=True,what=[],mpcLog=None,mheLog=None,simLog=None):
    assert isinstance(what,list)
    assert isinstance(names,list)
    
    fig = plt.figure()
    
    if title is None:
        if isinstance(names,str):
            title = names
        else:
            assert isinstance(names,list)
            if len(names) == 1:
                title = names[0]
            else:
                title = str(names)
    fig.canvas.set_window_title(str(title))

    plt.clf()
    
    n = len(names)
    for k,name in enumerate(names):
        plt.subplot(n,1,k+1)
        if 'mpc' in what:
            if mpcLog == None: raise Exception('you must provide a mpc log to plot its variables')
            mpcLog._plot(name,None,'k',when='all',showLegend=True)
        if 'sim' in what:
            if simLog == None: raise Exception('you must provide a sim log to plot its variables')
            simLog._plot(name,None,'',when=0,showLegend=True)
        if 'mhe' in what:
            if mheLog == None: raise Exception('you must provide a mhe log to plot its variables')
            N = mheLog._log['x'][0].shape[0]
            if not isinstance(name,list):
                name = [name]
            if name[0] in mheLog.xNames:
                print 'here'
                mheLog._plot(name,None,'o',when=N-1,showLegend=True)
            elif name[0] in mheLog.uNames:
                mheLog._plot(name,None,'o',when=N-2,showLegend=True)
        
        
    