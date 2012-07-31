import numpy
from numpy import cos,sin, vstack, hstack, multiply

casadiAvailable = False
casadiTypes = set()
try:
  import casadi as c
  casadiAvailable = True
  casadiTypes = set([type(c.SX()),type(c.SXMatrix())])
except ImportError:
  pass
  
def TRx(a):
  constr = numpy.matrix
  if casadiAvailable and type(a) in casadiTypes:
    constr = c.SXMatrix
  return  constr([[1,0,0,0],[0,cos(a),-sin(a),0],[0,sin(a),cos(a),0],[0,0,0,1]])

def TRy(a):
  constr = numpy.matrix
  if casadiAvailable and type(a) in casadiTypes:
    constr = c.SXMatrix
  return  constr([[cos(a),0,sin(a),0],[0,1,0,0],[-sin(a),0,cos(a),0],[0,0,0,1]])

def TRz(a):
  constr = numpy.matrix
  if casadiAvailable and type(a) in casadiTypes:
    constr = c.SXMatrix
  return  constr([[cos(a),-sin(a),0,0],[sin(a),cos(a),0,0],[0,0,1,0],[0,0,0,1]])

def tr(x,y,z):
  return  numpy.matrix([[1,0,0,x],[0,1,0,y],[0,0,1,z],[0,0,0,1]])
  
def Tquat(q0,q1,q2,q3):
  return R2T(quat(q0,q1,q2,q3))
  
def quat(q0,q1,q2,q3):
  """
  From Jeroen's presentation. q = [e*sin(theta/2); cos(theta/2)]
  """
  constr = numpy.matrix
  types =  set([type(q) for q in [q0,q1,q2,q3]])
  #if not(types.isdisjoint(casadiTypes)):
  #  constr = c.SXMatrix

  rho = constr([[q0],[q1],[q2]])
  rho_skew = skew(rho)
  I_3 = constr([[1.0,0,0],[0,1.0,0],[0,0,1.0]])

  A = multiply(I_3,(numpy.dot(rho.T,-rho)+q3*q3))+numpy.dot(rho,rho.T)*2.0-q3*rho_skew*2.0

  if not(types.isdisjoint(casadiTypes)):
    constr = c.SXMatrix

  return constr(A.T)

def quatOld(q0,q1,q2,q3):
  """
  From Shabana AA. Dynamics of multibody systems. Cambridge Univ Pr; 2005.
  defined as [ cos(theta/2) e*sin(theta/2) ]
  """
  constr = numpy.matrix
  types =  set([type(q) for q in [q0,q1,q2,q3]])
  #if not(types.isdisjoint(casadiTypes)):
  #  constr = c.SXMatrix
    
  E  = constr([[-q1, q0, -q3, q2],[-q2, q3, q0, -q1],[-q3,-q2,q1,q0]])
  Eb = constr([[-q1, q0, q3, -q2],[-q2, -q3, q0, q1],[-q3,q2,-q1,q0]])
  
  
  if not(types.isdisjoint(casadiTypes)):
    constr = c.SXMatrix
    
  return constr(numpy.dot(E,Eb.T))

def fullR(R_0_0,R_1_0,R_2_0, R_0_1, R_1_1, R_2_1, R_0_2, R_1_2, R_2_2):
  constr = numpy.matrix
  types =  set([type(q) for q in [R_0_0,R_1_0,R_2_0, R_0_1, R_1_1, R_2_1, R_0_2, R_1_2, R_2_2]])
  if not(types.isdisjoint(casadiTypes)):
    constr = c.SXMatrix
  return constr([[R_0_0,  R_0_1,  R_0_2],[R_1_0,  R_1_1,  R_1_2 ],[R_2_0,  R_2_1,  R_2_2 ]])
  
def TfullR(R_0_0,R_1_0,R_2_0, R_0_1, R_1_1, R_2_1, R_0_2, R_1_2, R_2_2):
  return R2T(fullR(R_0_0,R_1_0,R_2_0, R_0_1, R_1_1, R_2_1, R_0_2, R_1_2, R_2_2))
  

def origin() :
  return tr(0,0,0)
  
  
def trp(T):
  return numpy.matrix(T)[:3,3]
  
def inv(T):
  R=numpy.matrix(T2R(T).T)
  constr = numpy.matrix
  if type(T) in casadiTypes:
    constr = c.SXMatrix
  return constr(vstack((hstack((R,-numpy.dot(R,trp(T)))),numpy.matrix([0,0,0,1]))))


def vectorize(vec):
  """
  Make sure the result is something you can index with single index
  """
  if hasattr(vec,"shape"):
    if vec.shape[0] > 1 and vec.shape[1] > 1:
      raise Exception("vectorize: got real matrix instead of vector like thing: %s" % str(vec))
    if vec.shape[1] > 1:
      vec = vec.T
    if hasattr(vec,"tolist"):
      vec = [ i[0] for i in vec.tolist()]
  return vec

def skew(vec):
  myvec = vectorize(vec)

  x = myvec[0]
  y = myvec[1]
  z = myvec[2]

  constr = numpy.matrix
  types =  set([type(q) for q in [x,y,z]])
  if not(types.isdisjoint(casadiTypes)):
    constr = c.SXMatrix

  return constr([[0,-z,y],[z,0,-x],[-y,x,0]])
  
def invskew(S):
  return c.SXMatrix([S[2,1],S[0,2],S[1,0]])
  
def cross(a,b):
  return c.mul(skew(a),b)

def T2R(T):
  """
   Rotational part of transformation matrix 
   
  """
  return T[0:3,0:3]
  
def R2T(R):
  """
   Pack a rotational matrix in a homogenous form
   
  """
  constr = numpy.matrix
  if type(R) in casadiTypes:
    constr = c.SXMatrix
  T  = constr([[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,1.0]])
  T[:3,:3] = R
  return T
  
def T2w():
  """
   skew(w_100) = T2w(T_10)
   
  """
  
def T2W(T,p,dp):
  """
   w_101 = T2W(T_10,p,dp)
   
  """
  R = T2R(T)
  dR = c.reshape(c.mul(c.jacobian(R,p),dp),(3,3))
  return invskew(c.mul(R.T,dR))

def quatDynamics(q0,q1,q2,q3):
  """
   dot(q) = quatDynamics(q)*w_101
   
  """
  B = numpy.matrix([[q3,-q2,q1],[q2,q3,-q0],[-q1,q0,q3],[-q0,-q1,-q2]])*0.5
  return B

def T2WJ(T,p):
  """
   w_101 = T2WJ(T_10,p).diff(p,t)
   
  """
  R = T2R(T)
  RT = R.T
  
  temp = []
  for i,k in [(2,1),(0,2),(1,0)]:
     #temp.append(c.mul(c.jacobian(R[:,k],p).T,R[:,i]).T)
     temp.append(c.mul(RT[i,:],c.jacobian(R[:,k],p)))

  return c.vertcat(temp)
  
