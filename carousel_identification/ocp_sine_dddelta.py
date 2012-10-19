# -*- coding: utf-8 -*-
"""
Created on Thu Sep  6 17:26:27 2012

@author: kurt
"""
from casadi import *
import numpy as NP
import matplotlib.pyplot as plt
import time
from casadi.tools import *
from kinetics.flatkinematics import skew
from scipy.linalg.matfuncs import sqrtm
from scipy.linalg.matfuncs import toreal
plt.interactive(True)
if 1:
    alpha = ssym("alpha")
    beta = ssym("beta")
    R = ssym("R")
    rIMU = ssym("rIMU")
    pIMU = ssym("pIMU")
    yIMU = ssym("yIMU")
    
    A_dddelta = ssym("A_dddelta") # amplitude of variation of dddelta
    P_dddelta = ssym("P_dddelta") # phase of variation of dddelta
    
    g = NP.loadtxt('g.dat')
    
    X = vertcat([alpha,beta,R,rIMU,pIMU,yIMU])
    
    delta = ssym("delta")
    ddelta =ssym("ddelta")
    dddelta =ssym("dddelta")
    
    aIMU = vertcat([ g*cos(beta)*(sin(rIMU)*sin(yIMU) + cos(rIMU)*cos(yIMU)*sin(pIMU)) - (R*dddelta + g*sin(alpha + delta)*sin(beta))*(cos(rIMU)*sin(yIMU) - cos(yIMU)*sin(pIMU)*sin(rIMU)) - cos(pIMU)*cos(yIMU)*(R*ddelta**2 + g*sin(beta)*cos(alpha + delta)) ,\
                     (R*dddelta + g*sin(alpha + delta)*sin(beta))*(cos(rIMU)*cos(yIMU) + sin(pIMU)*sin(rIMU)*sin(yIMU)) - g*cos(beta)*(cos(yIMU)*sin(rIMU) - cos(rIMU)*sin(pIMU)*sin(yIMU)) - cos(pIMU)*sin(yIMU)*(R*ddelta**2 + g*sin(beta)*cos(alpha + delta)) ,\
                     sin(pIMU)*(R*ddelta**2 + g*sin(beta)*cos(alpha + delta)) + cos(pIMU)*sin(rIMU)*(R*dddelta + g*sin(alpha + delta)*sin(beta)) + g*cos(beta)*cos(pIMU)*cos(rIMU) ])
 
    wIMU = vertcat([ ddelta*(sin(rIMU)*sin(yIMU) + cos(rIMU)*cos(yIMU)*sin(pIMU)) ,\
                     -ddelta*(cos(yIMU)*sin(rIMU) - cos(rIMU)*sin(pIMU)*sin(yIMU)) ,\
                     ddelta*cos(pIMU)*cos(rIMU) ])
    
    
    h = SXFunction([X,delta,ddelta,dddelta],[aIMU,wIMU])
    h.init()
    
    aIMU_meas = NP.loadtxt('aIMU.dat')
    wIMU_meas = NP.loadtxt('wIMU.dat')
    delta_meas = NP.loadtxt('delta.dat')
    
    Nmeas = aIMU_meas.shape[0]
    N = 20
    Nmeas_i = Nmeas/N
    aIMU_meas = aIMU_meas[0:N*Nmeas_i,:]
    wIMU_meas = wIMU_meas[0:N*Nmeas_i,:]
    delta_meas = delta_meas[0:N*Nmeas_i]
    Nmeas = N*Nmeas_i
    delta_MS = ssym("delta_MS",N,1)
    ddelta_MS = ssym("delta_MS",N,1)
    dddelta_MS = ssym("dddelta_MS",N,1)
    
    ddelta_i = SXMatrix(N,Nmeas_i)
    delta_i = SXMatrix(N,Nmeas_i)
    
    delta_i[:,0] = delta_MS
    ddelta_i[:,0] = ddelta_MS
    dt = 0.002
    for i in range(1,Nmeas_i):
        ddelta_i[:,i] = ddelta_i[:,i-1] + dt*(dddelta_MS[i/Nmeas_i] + A_dddelta*sin(delta_i[:,i]+P_dddelta))
        delta_i[:,i] = delta_i[:,i-1] + dt*ddelta_i[:,i-1] + dt**2/2*(dddelta_MS[i/Nmeas_i] + A_dddelta*sin(delta_i[:,i]+P_dddelta))
    h_delta = SXFunction([delta_MS,ddelta_MS,dddelta_MS,A_dddelta,P_dddelta],[delta_i])
    h_delta.init()
    h_ddelta = SXFunction([ddelta_MS,dddelta_MS,A_dddelta,P_dddelta],[ddelta_i])
    h_ddelta.init()
    obj = 0
    for i in range(Nmeas):
        obj += mul((aIMU_meas[i,:].T - h.eval([X,delta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],ddelta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],dddelta_MS[i/Nmeas_i,0]])[0]).T,(aIMU_meas[i,:].T - h.eval([X,delta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],ddelta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],dddelta_MS[i/Nmeas_i,0]])[0]))
        obj += mul((wIMU_meas[i,:].T - h.eval([X,delta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],ddelta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],dddelta_MS[i/Nmeas_i,0]])[1]).T,(wIMU_meas[i,:].T - h.eval([X,delta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],ddelta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i],dddelta_MS[i/Nmeas_i,0]])[1]))
        obj += (delta_i[i/Nmeas_i,i-(i/Nmeas_i)*Nmeas_i]-delta_meas[i])**2
    Xall = vertcat([X,delta_MS,ddelta_MS,dddelta_MS,A_dddelta,P_dddelta])
    f_obj = SXFunction([Xall],[obj])
    f_obj.setOption('numeric_hessian',True)
    f_obj.init()
    g = [];
    for i in range(1,N):
        g.append(delta_MS[i,0] - ( delta_i[i-1,-1] + dt*ddelta_i[i-1,-1] + dt**2/2*dddelta_MS[i-1] ))
        g.append(ddelta_MS[i,0] - ( ddelta_i[i-1,-1] + dt*dddelta_MS[i-1] ))
    G = SXFunction([Xall],[g])
    G.init()
    
    nlp_solver = IpoptSolver(f_obj,G)
    nlp_solver.setOption("max_iter",3000)
    #nlp_solver.setOption("generate_hessian",True)
    #nlp_solver.setOption('linear_solver','MA57')
    nlp_solver.init()
    nlp_solver.input(NLP_X_INIT)[0] = 0; # alpha
    nlp_solver.input(NLP_X_INIT)[1] = 0; # beta
    nlp_solver.input(NLP_X_INIT)[2] = 1; # R
    nlp_solver.input(NLP_X_INIT)[3] = 0; # rIMU
    nlp_solver.input(NLP_X_INIT)[4] = 0; # pIMU
    nlp_solver.input(NLP_X_INIT)[5] = 0; # yIMU
    for i in range(N):
        nlp_solver.input(NLP_X_INIT)[6+i] = delta_meas[i*Nmeas_i]; # delta_0
        nlp_solver.input(NLP_X_INIT)[6+N+i] = NP.loadtxt("ddelta.dat"); # ddelta_0
        nlp_solver.input(NLP_X_INIT)[6+2*N+i] = 0.0; # dddelta
    nlp_solver.input(NLP_X_INIT)[6+3*N] = 0.0;
    nlp_solver.input(NLP_X_INIT)[6+3*N+1] = 0.0;
    nlp_solver.input(NLP_LBG).set(DMatrix.zeros(2*(N-1),1))
    nlp_solver.input(NLP_UBG).set(DMatrix.zeros(2*(N-1),1))
    nlp_solver.solve()
    print 'Finished solving'
    Xsol = nlp_solver.output(NLP_X_OPT)
    alpha = Xsol[0]
    beta = Xsol[1]
    R = Xsol[2]
    rIMU = Xsol[3]
    pIMU = Xsol[4]
    yIMU = Xsol[5]
    delta_MS_sol = Xsol[6:6+N]
    ddelta_MS_sol = Xsol[6+N:6+2*N]
    dddelta_MS_sol = Xsol[6+2*N:6+3*N]
    A_dddelta_sol = Xsol[6+3*N]
    P_dddelta_sol = Xsol[6+3*N+1]
    print 'got solution'
    res = jacobian(obj,Xall)
    f = SXFunction([Xall],[res])  
    f.setOption("numeric_jacobian",True)
    f.init()
    jac = Jacobian ( f )
    jac.init()
    jac.input(0).set(Xsol)
    print 'computing hessian'
    jac.evaluate()
    H = jac.output( 0 )
    print 'hessian computed'

aIMU_pred = DMatrix(delta_meas.shape[0],3)
wIMU_pred = DMatrix(delta_meas.shape[0],3)
print 'computing prediction'
h_delta.input(0).set(delta_MS_sol)
h_delta.input(1).set(ddelta_MS_sol)
h_delta.input(2).set(dddelta_MS_sol)
h_delta.input(3).set(A_dddelta_sol)
h_delta.input(4).set(P_dddelta_sol)
h_delta.evaluate()
h_ddelta.input(0).set(ddelta_MS_sol)
h_ddelta.input(1).set(dddelta_MS_sol)
h_ddelta.input(2).set(A_dddelta_sol)
h_ddelta.input(3).set(P_dddelta_sol)
h_ddelta.evaluate()
print 'delta computed'

for i in range(Nmeas):
    h.input(0).set(Xsol[0:6])
    h.input(1).set(h_delta.output(0)[i/Nmeas_i,i-Nmeas_i*(i/Nmeas_i)])
    h.input(2).set(h_ddelta.output(0)[i/Nmeas_i,i-Nmeas_i*(i/Nmeas_i)])
    h.input(3).set(dddelta_MS_sol[i/Nmeas_i]+A_dddelta_sol*sin(h_delta.output(0)[i/Nmeas_i,i-Nmeas_i*(i/Nmeas_i)]+P_dddelta_sol))
    h.evaluate()
    aIMU_pred[i,:] = h.output(0).T
    wIMU_pred[i,:] = h.output(1).T
print 'acc computed'
aIMU_res = aIMU_pred-aIMU_meas
wIMU_res = wIMU_pred-wIMU_meas
print 'Saving data'
NP.savetxt('wIMU_res.dat',wIMU_res)
NP.savetxt('aIMU_res.dat',aIMU_res)
NP.savetxt('aIMU_pred.dat',aIMU_pred)
NP.savetxt('wIMU_pred.dat',wIMU_pred)
NP.savetxt('hessian.dat',H)
NP.savetxt('XSol.dat',Xsol)
print 'Data saved'
#tgrid = NP.linspace(0,0.002*(delta_meas.shape[0]-1),delta_meas.shape[0])
#plt.figure(1)
#plt.clf()
#plt.plot(tgrid,aIMU_pred[:,0],'b--')
#plt.plot(tgrid,aIMU_meas[:,0],'r--')
#plt.plot(tgrid,aIMU_pred[:,1],'b-')
#plt.plot(tgrid,aIMU_meas[:,1],'r-')
#plt.plot(tgrid,aIMU_pred[:,2],'b-.')
#plt.plot(tgrid,aIMU_meas[:,2],'r-.')
#plt.title("Acceleration meaured vs predicted")
#plt.legend(['ax_predicted','ax_measured','ay_predicted','ay_measured','az_predicted','az_measured'])
#plt.xlabel('time')
#plt.grid()
#plt.figure(2)
#plt.clf()
#plt.plot(tgrid,wIMU_pred[:,0],'b--')
#plt.plot(tgrid,wIMU_meas[:,0],'r--')
#plt.plot(tgrid,wIMU_pred[:,1],'b-')
#plt.plot(tgrid,wIMU_meas[:,1],'r-')
#plt.plot(tgrid,wIMU_pred[:,2],'b-.')
#plt.plot(tgrid,wIMU_meas[:,2],'r-.')
#plt.title("Angular velocities meaured vs predicted")
#plt.legend(['wx_predicted','wx_measured','wy_predicted','wy_measured','wz_predicted','wz_measured'])
#plt.xlabel('time')
#plt.grid()
#
#plt.figure(3)
#plt.clf()
#plt.subplot(311)
#plt.title("Acceleration residu")
#plt.plot(tgrid,aIMU_res[:,0])
#plt.ylabel('a_x')
#plt.subplot(312)
#plt.plot(tgrid,aIMU_res[:,1])
#plt.ylabel('a_y')
#plt.subplot(313)
#plt.plot(tgrid,aIMU_res[:,2])
#plt.ylabel('a_z')
#plt.xlabel('time')
#plt.grid()
#plt.figure(4)
#plt.clf()
#plt.subplot(311)
#plt.plot(tgrid,wIMU_res[:,0])
#plt.title("Angular velocity residu")
#plt.ylabel('omega_x')
#plt.subplot(312)
#plt.plot(tgrid,wIMU_res[:,1])
#plt.ylabel('omega_y')
#plt.subplot(313)
#plt.plot(tgrid,wIMU_res[:,2])
#plt.ylabel('omega_z')
#plt.xlabel('time')
#plt.grid()
#plt.figure(5)
#plt.clf()
#plt.subplot(211)
#plt.plot(tgrid,delta_meas,'b')
#plt.plot(tgrid,h_delta.output(0),'r')
#plt.ylabel('delta')
#plt.subplot(212)
#plt.plot(tgrid,NP.loadtxt('ddelta_t.dat'),'b')
#plt.plot(tgrid,h_ddelta.output(0),'r')
#plt.ylabel('ddelta')
#plt.xlabel('time')
#plt.grid()
