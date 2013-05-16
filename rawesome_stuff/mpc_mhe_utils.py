# -*- coding: utf-8 -*-
"""
Created on Wed Apr 24 15:19:57 2013

@author: mzanon
"""
import numpy as np
import scipy
#import copy
import matplotlib.pyplot as plt

#import casadi as C
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
    
def UpdateArrivalCost(integrator, x, u, xL, yL, PL, VL, WL): 
    ''' Arrival cost implementation.
        Approximate the solution of:
        min_{xL_,uL_,xL1_} ||  PL ( xL_-xL )         ||
                           ||  VL ( yL-h(xL_,uL_) )  ||
                           ||  WL wx                 ||
                     s.t.  wx = xL1_ - f(xL_,uL_)
        
        Linearization (at the last MHE estimate x,u which is different from xL,uL):
        f(xL_,uL_) ~= f(x,u) + df(x,u)/dx (xL_-x) + df(x,u)/du (uL_-u)
                   ~= f(x,u) +         Xx (xL_-x) +         Xu (uL_-u)
                   ~= f(x,u) - Xx x - Xu u + Xx xL_ + Xu uL_
                   ~= x_tilde              + Xx xL_ + Xu uL_
        h(xL_,uL_) ~= h(x,u) + dh(x,u)/dx (xL_-x) + dh(x,u)/du (uL_-u)
                   ~= f(x,u) +         Hx (xL_-x) +         Hu (uL_-u)
                   ~= h(x,u) - Hx x - Hu u + Hx xL_ + Hu uL_
                   ~= h_tilde              + Hx xL_ + Hu uL_
                   
        Linearized problem:
        min_{xL_,uL_,xL1_} ||  PL ( xL_ - xL )                          ||
                           ||  VL ( yL - h_tilde - Hx xL_ - Hu uL_ )    ||
                           ||  WL ( xL1_ - x_tilde - Xx xL_ - Xu uL_ )  ||
        
        Rewrite as:
        min_{xL_,uL_,xL1_} ||  M ( xL_, uL_, xL1_ ) + res  ||
        
        After QR factorization of M:
        min_{xL_,uL_,xL1_} ||  R ( xL_, uL_, xL1_ ) + rho  ||
        '''
    nx = x.shape[0]
    nu = u.shape[0]
    nV = VL.shape[0]
    
    integrator.x = x
    integrator.u = u
    out = integrator.getOutputs()
#    h = np.squeeze(out['measurements'])
    x1 = integrator.step()
    Xx = integrator.dx1_dx0
    Xu = integrator.dx1_du
    
    Hx = np.append(np.eye(nx),np.zeros((4,25)),axis=0)
    Hu = np.append(np.zeros((25,4)),np.eye(nu),axis=0)
    
    x_tilde = x1 - np.dot(Xx,x) - np.dot(Xu,u)
    h_tilde =  h - np.dot(Hx,x) - np.dot(Hu,u)
    
    res = np.bmat([ -np.dot(PL, xL),
                     np.dot(VL, yL - h_tilde),
                    -np.dot(WL, x_tilde) ])
    res = np.squeeze(np.array(res))
    
    M = np.bmat([[             PL,  np.zeros((nx,nu)), np.zeros((nx,nx)) ],
                 [ -np.dot(VL,Hx),     -np.dot(VL,Hu), np.zeros((nV,nx)) ],
                 [ -np.dot(WL,Xx),     -np.dot(WL,Xu),                WL ]])
    
    Q, R = np.linalg.qr(M)
    
#    R1  = R[:nx+nu,:nx+nu]
#    R12 = R[:nx+nu,nx+nu:]
    R2  = R[nx+nu:,nx+nu:]
    
#    rho = np.linalg.solve(Q,res)
    rho = np.squeeze(np.array(np.dot(Q.T,res)))
    rho2 = rho[nx+nu:]
    
    PL1 = R2
    xL1 = -np.linalg.solve(R2,rho2)
    
    return PL1, xL1
    
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

def InitializeMPC(mpcrt,Rint,dae,conf,refP):
        
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
    
    mpcrt.Q = Q
    mpcrt.R = R
    
#    # Linearize the system at the reference
#    nx = Rint.x.shape[0]
#    Rint.x = mpcrt.yN
#    Rint.u = mpcrt.y[-1,nx:]
#    Rint.step()
#    A = Rint.dx1_dx0
#    B = Rint.dx1_du
#    
#    # Compute the LQR
#    K,P = dlqr(A, B, Q, R, N=None)
    
    P = np.eye(Q.shape[0])*10
    mpcrt.SN = P
    
#    mpcrt.S  = np.eye(25+4)
#    mpcrt.SN = np.eye(25)
    
#    mpcLog = rawe.ocp.ocprt.Logger(mpcrt,dae)
#    
#    return mpcLog

#blah
#class MpcMhe(object):
#    def __init__(self):
#        blah
#    def initializeMhe(self):
#        raise Exception('you must override this')
#        
#class KiteMpcMhe(MpcMhe):
#    def __init__(self,userdata):
#        MpcMhe.__init__(self)
#        self.myydata = userdata
#        
#    def initializeMhe(self):
#        print self.userdata

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
        
    mhert.x0 = Xref[0,:]
    # Set the initial guess
    mhert.x = Xref
    mhert.u = Uref
    
    # Set the measurements (temporary)
    mhert.y = np.append(Xref[:-1,:],Uref,axis=1)
    mhert.yN = Xref[-1,:]
    
    # Set the covariance (temporary)
    mhert.S  = np.eye(mhert.y.shape[1])
    mhert.S[25:,25:] = np.eye(4)*1
    mhert.SN = np.eye(mhert.yN.shape[0])
    
#    mheLog = rawe.ocp.ocprt.Logger(mhert,dae)
#    
#    return mheLog
    
def SimulateAndShift(mpcRT,mheRT,sim,simLog,Rint,dae,conf,refP):

    mheRT.log()    
    mpcRT.log()
    
    # Get the measurement BEFORE simulating
#    outs = sim.getOutputs(mpcRT.x[0,:],mpcRT.u[0,:],{})
#    new_y  = np.squeeze(outs['measurements'])
    new_y = np.append(mpcRT.x[0,:],mpcRT.u[0,:]) #+ np.random.randn(29)*0.001
    # Simulate the system
    new_x = sim.step(mpcRT.x[0,:],mpcRT.u[0,:],{})
    # Get the last measurement AFTER simulating
    new_out = sim.getOutputs(new_x,mpcRT.u[0,:],{})
#    new_yN = np.array([outs['measurementsN']])
    new_yN = np.squeeze(new_x) #+ np.random.randn(25)*0.001
    
    simLog.log(new_x=new_x,new_y=new_y,new_yN=new_yN,new_out=new_out)
    
    # Linearize the system at the reference
    nx = Rint.x.shape[0]
#    Rint.x = mpcRT.yN
#    Rint.u = mpcRT.y[-1,nx:]
#    Rint.step()
#    A = Rint.dx1_dx0
#    B = Rint.dx1_du
#    Q = mpcRT.Q
#    R = mpcRT.R
#    # Compute the LQR
#    K,P = dlqr(A, B, Q, R, N=None)
    
    xN = mpcRT.x[-1,:]
#    uN = -np.dot(K,xN)
    uN = mpcRT.y[-1,nx:]
    Rint.x = xN
    Rint.u = uN
    Rint.step()
    
    new_xMPC = [Rint.x]
    new_uMPC = [uN]
    
    cos_delta,sin_delta = mpcRT.y[1,nx-2:nx]
    
    delta0 = np.arctan2(sin_delta,cos_delta)
    
    xref, uref = GenerateReference(dae,conf,refP)
    
    N = mpcRT.u.shape[0]
    Xref = [xref[name] for name in dae.xNames()]
    Uref = [uref[name] for name in dae.uNames()]
    Xref = np.array( [Xref]*(N+1) )
    Uref = np.array( [Uref]*N     )
    
    ts = mpcRT._ts
    # The only part that changes is cos(delta), sin(delta) 
    CS = np.array([ [np.cos(k*ts*refP['ddelta0'] + delta0), np.sin(k*ts*refP['ddelta0'] + delta0)] for k in range(-N,1) ])
    for k,name in enumerate(dae.xNames()):
        if name == 'cos_delta': Xref[:,k] = CS[:,0]
        if name == 'sin_delta': Xref[:,k] = CS[:,1]
    
    new_yMPC = np.append(Xref[-2,:],Uref[-1,:])
    new_yNMPC = Xref[-1,:]
        
    # shift
    mpcRT.shift(new_x=new_xMPC,new_u=new_uMPC,new_y=new_yMPC,new_yN=new_yNMPC)
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
                mheLog._plot(name,None,'o',when=N-1,showLegend=True)
            elif name[0] in mheLog.uNames:
                mheLog._plot(name,None,'o',when=N-2,showLegend=True)
        
        
    