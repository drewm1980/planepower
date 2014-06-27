import math

from numpy import *

def TRx(a):
  return  matrix([[1,0,0,0],[0,cos(a),-sin(a),0],[0,sin(a),cos(a),0],[0,0,0,1]])

def TRy(a):
  return  matrix([[cos(a),0,sin(a),0],[0,1,0,0],[-sin(a),0,cos(a),0],[0,0,0,1]])

def TRz(a):
  return  matrix([[cos(a),-sin(a),0,0],[sin(a),cos(a),0,0],[0,0,1,0],[0,0,0,1]])

def tr(x,y,z):
  return  matrix([[1,0,0,x],[0,1,0,y],[0,0,1,z],[0,0,0,1]]);

def origin() :
  return tr(0,0,0)
  
def NotImplemented(*args,**kwargs):
  raise Exception("Not implemented in kinetics.flatkinematics")

Tquat  = full   = TfullR = fullR =  NotImplemented
