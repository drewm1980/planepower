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
if 0:
    alpha = ssym("alpha")
    beta = ssym("beta")
    omega = ssym("omega")
    R = ssym("R")
    rIMU = ssym("rIMU")
    pIMU = ssym("pIMU")
    yIMU = ssym("yIMU")
    
    
    g = NP.loadtxt('g.dat')
    omega = NP.loadtxt('ddelta.dat')
    
    X = vertcat([alpha,beta,R,rIMU,pIMU,yIMU])
    
    delta = ssym("delta")
    
    
    aIMU = vertcat([ g*cos(beta)*(sin(rIMU)*sin(yIMU) + cos(rIMU)*cos(yIMU)*sin(pIMU)) - cos(pIMU)*cos(yIMU)*(R*omega**2 + g*cos(alpha + delta)*sin(beta)) - g*sin(alpha + delta)*sin(beta)*(cos(rIMU)*sin(yIMU) - cos(yIMU)*sin(pIMU)*sin(rIMU)) ,\
                     g*sin(alpha + delta)*sin(beta)*(cos(rIMU)*cos(yIMU) + sin(pIMU)*sin(rIMU)*sin(yIMU)) - g*cos(beta)*(cos(yIMU)*sin(rIMU) - cos(rIMU)*sin(pIMU)*sin(yIMU)) - cos(pIMU)*sin(yIMU)*(R*omega**2 + g*cos(alpha + delta)*sin(beta)) ,\
                     sin(pIMU)*(R*omega**2 + g*cos(alpha + delta)*sin(beta)) + g*cos(beta)*cos(pIMU)*cos(rIMU) + g*sin(alpha + delta)*cos(pIMU)*sin(beta)*sin(rIMU) ])
    wIMU = vertcat([ omega*(sin(rIMU)*sin(yIMU) + cos(rIMU)*cos(yIMU)*sin(pIMU)) ,\
                     -omega*(cos(yIMU)*sin(rIMU) - cos(rIMU)*sin(pIMU)*sin(yIMU)) ,\
                     omega*cos(pIMU)*cos(rIMU) ])
    
    
    h = SXFunction([X,delta],[aIMU,wIMU])
    h.init()
    
    aIMU_meas = NP.loadtxt('aIMU.dat')
    wIMU_meas = NP.loadtxt('wIMU.dat')
    delta_meas = NP.loadtxt('delta.dat')
    
    obj = 0
    for i in range(aIMU_meas.shape[0]):
        obj += mul((aIMU_meas[i,:].T - h.eval([X,delta_meas[i]])[0]).T,(aIMU_meas[i,:].T - h.eval([X,delta_meas[i]])[0]))
        obj += mul((wIMU_meas[i,:].T - h.eval([X,delta_meas[i]])[1]).T,(wIMU_meas[i,:].T - h.eval([X,delta_meas[i]])[1]))
    f_obj = SXFunction([X],[obj])
    f_obj.setOption('numeric_hessian',True)
    f_obj.init()
    
    nlp_solver = IpoptSolver(f_obj)
    #nlp_solver.setOption("generate_hessian",True)
    #nlp_solver.setOption('linear_solver','MA57')
    nlp_solver.init()
    nlp_solver.input(NLP_X_INIT)[0] = 0; # alpha
    nlp_solver.input(NLP_X_INIT)[1] = 0; # beta
    nlp_solver.input(NLP_X_INIT)[2] = 1; # R
    nlp_solver.input(NLP_X_INIT)[3] = 0; # rIMU
    nlp_solver.input(NLP_X_INIT)[4] = 0; # pIMU
    nlp_solver.input(NLP_X_INIT)[5] = 0; # yIMU
    
    nlp_solver.solve()
    
    Xsol = nlp_solver.output(NLP_X_OPT)
    alpha = Xsol[0]
    beta = Xsol[1]
    R = Xsol[2]
    rIMU = Xsol[3]
    pIMU = Xsol[4]
    yIMU = Xsol[5]
    
    res = jacobian(obj,X)
    f = SXFunction([X],[res])  
    f.setOption("numeric_jacobian",True)
    f.init()
    jac = Jacobian ( f )
    jac.init()
    jac.input(0).set(Xsol)
    jac.evaluate()
    H = jac.output( 0 )
    cov = inv(H)
    err = sqrt(diag(cov))/Xsol

aIMU_pred = DMatrix(delta_meas.shape[0],3)
wIMU_pred = DMatrix(delta_meas.shape[0],3)

for i in range(delta_meas.shape[0]):
    h.input(0).set(Xsol)
    h.input(1).set(delta_meas[i])
    h.evaluate()
    aIMU_pred[i,:] = h.output(0).T
    wIMU_pred[i,:] = h.output(1).T
tgrid = NP.linspace(0,0.002*(delta_meas.shape[0]-1),delta_meas.shape[0])
plt.figure(1)
plt.clf()
plt.plot(tgrid,aIMU_pred[:,0],'b--')
plt.plot(tgrid,aIMU_meas[:,0],'r--')
plt.plot(tgrid,aIMU_pred[:,1],'b-')
plt.plot(tgrid,aIMU_meas[:,1],'r-')
plt.plot(tgrid,aIMU_pred[:,2],'b-.')
plt.plot(tgrid,aIMU_meas[:,2],'r-.')
plt.title("Acceleration meaured vs predicted")
plt.legend(['ax_predicted','ax_measured','ay_predicted','ay_measured','az_predicted','az_measured'])
plt.xlabel('time')
plt.grid()
plt.figure(2)
plt.clf()
plt.plot(tgrid,wIMU_pred[:,0],'b--')
plt.plot(tgrid,wIMU_meas[:,0],'r--')
plt.plot(tgrid,wIMU_pred[:,1],'b-')
plt.plot(tgrid,wIMU_meas[:,1],'r-')
plt.plot(tgrid,wIMU_pred[:,2],'b-.')
plt.plot(tgrid,wIMU_meas[:,2],'r-.')
plt.title("Angular velocities meaured vs predicted")
plt.legend(['wx_predicted','wx_measured','wy_predicted','wy_measured','wz_predicted','wz_measured'])
plt.xlabel('time')
plt.grid()

aIMU_res = aIMU_pred-aIMU_meas
wIMU_res = wIMU_pred-wIMU_meas

plt.figure(3)
plt.clf()
plt.subplot(311)
plt.title("Acceleration residu")
plt.plot(tgrid,aIMU_res[:,0])
plt.ylabel('a_x')
plt.subplot(312)
plt.plot(tgrid,aIMU_res[:,1])
plt.ylabel('a_y')
plt.subplot(313)
plt.plot(tgrid,aIMU_res[:,2])
plt.ylabel('a_z')
plt.xlabel('time')
plt.grid()
plt.figure(4)
plt.clf()
plt.subplot(311)
plt.plot(tgrid,wIMU_res[:,0])
plt.title("Angular velocity residu")
plt.ylabel('omega_x')
plt.subplot(312)
plt.plot(tgrid,wIMU_res[:,1])
plt.ylabel('omega_y')
plt.subplot(313)
plt.plot(tgrid,wIMU_res[:,2])
plt.ylabel('omega_z')
plt.xlabel('time')
plt.grid()