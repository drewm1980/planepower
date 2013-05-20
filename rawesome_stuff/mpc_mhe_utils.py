# -*- coding: utf-8 -*-
"""
Created on Wed Apr 24 15:19:57 2013

@author: mzanon
"""
import numpy as np
import scipy
import matplotlib.pyplot as plt

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
    
    for k in range(Xref.shape[0]-1):
        integrator.x = Xref[k,:]
        integrator.u = Uref[k,:]
        integrator.step()
        mpcrt.z[k,:] = integrator.z
        
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
    R = [1.0*1e6]*4
       
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

def InitializeMHE(mhert,integrator,dae,conf,refP,Covariance):
    
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
    
    for k in range(Xref.shape[0]-1):
        integrator.x = Xref[k,:]
        integrator.u = Uref[k,:]
        integrator.step()
        mhert.z[k,:] = integrator.z
    
    # Set the measurements
    Yref = []
    for k in range(Xref.shape[0]-1):
        Yref.append(mhert.getY(Xref[k,:],Uref[k,:]))
    mhert.y = np.array(Yref)
    mhert.yN = mhert.getYN(Xref[-1,:])
    
    # Set the covariance (temporary)
    Cov = np.array([])
    for name in mhert.measNames:
        Cov = np.append( Cov, np.ones(dae[name].shape)*Covariance[name] )
    CovEnd = np.array([])
    for name in mhert.endMeasNames:
        CovEnd = np.append( CovEnd, np.ones(dae[name].shape)*Covariance[name] )
    
    mhert.S  = np.diag(Cov)
#    mhert.S[25:,25:] = np.eye(4)*1
    mhert.SN = np.diag(CovEnd)
    
#    mheLog = rawe.ocp.ocprt.Logger(mhert,dae)
#    
#    return mheLog
    
def SimulateAndShift(mpcRT,mheRT,sim,Rint,dae,conf,refP):

    mheRT.log()    
    mpcRT.log()
    
    new_u = mpcRT.u[0,:]
    # Get y BEFORE simulating
    new_y = mheRT.getY(sim._log['x'][-1],new_u) #+ np.random.randn(29)*0.001
    # Simulate the system
    new_x = sim.step(sim._log['x'][-1],new_u,{})
    # Get the last measurement AFTER simulating
    new_out = sim.getOutputs(new_x,new_u,{})
#    new_yN = np.array([outs['measurementsN']])
    new_yN = mheRT.getYN(new_x) #+ np.random.randn(25)*0.001
    
    sim.log(new_x=new_x,new_u=new_u,new_y=new_y,new_yN=new_yN,new_out=new_out)
    mheRT.shift(new_x=[mpcRT.x[1,:]],new_u=[mpcRT.u[1,:]],new_y=new_y,new_yN=new_yN)
    
    # Linearize the system at the reference
    nx = Rint.x.shape[0]
    
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
    
    
def InitializeSim(dae,intType,ts,intOpts):
    if intType == 'Idas':
        sim = rawe.sim.Sim(dae,ts)
    elif intType == 'RtIntegrator':
        sim = rawe.RtIntegrator(dae,ts=ts,options=intOpts)
    else:
        raise Exception('integrator not supported')
    
#    simLog = SimLog(dae,sim)
    
    return sim
    
        
class Plotter(object):
    def __init__(self,simLog,mheRT,mpcRT):
        self._simLog = simLog
        self._mheLog  = mheRT
        self._mpcLog  = mpcRT
        
    def _plot(self,name,title,what,mheHorizon=False):
        if 'mpc' in what:
            self._mpcLog._plot(name,title,'k',when='all',showLegend=True)
        if 'sim' in what:
            self._simLog._plot(name,title,'',when=0,showLegend=True)
        if 'mhe' in what:
            N = self._mheLog._log['x'][0].shape[0] - 1
            if not isinstance(name,list):
                name = [name]
            if name[0] in self._mheLog.xNames():
                self._mheLog._plot(name,title,'o',when=N,showLegend=True)
                if mheHorizon:
                    print "plotting mhe horiz x"
                    self._mheLog._plot(name,title,'',when='all',offset='mhe',showLegend=False)
            elif name[0] in self._mheLog.uNames():
                self._mheLog._plot(name,title,'o',when=N-1,showLegend=True)
                if mheHorizon:
                    print "plotting mhe horiz u"
                    self._mheLog._plot(name,title,'',when='all',offset='mhe',showLegend=False)
            else:
                self._mheLog._plot(name,title,'o',showLegend=True)
                if mheHorizon:
                    print "plotting mhe horiz other"
                    self._mheLog._plot(name,title,'',when='all',offset='mhe',showLegend=False)
                
    def plot(self,names,title=None,showLegend=True,mheHorizon=False,what=[]):
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
        self._plot(names,title,what,mheHorizon=mheHorizon)
            
    def subplot(self,names,title=None,showLegend=True,mheHorizon=False,what=[]):
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
            self._plot(name,title,what,mheHorizon=mheHorizon)
