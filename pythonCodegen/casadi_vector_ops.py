from casadi import vertcat

def dot(a,b):
    c = a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
    return c

def cross(a,b):
    w1=a[1]*b[2]-a[2]*b[1]
    w2=a[2]*b[0]-a[0]*b[2]
    w3=a[0]*b[1]-a[1]*b[0]
    c = vertcat([w1,w2,w3])
    return c

