#!/usr/bin/env python
from casadi import *
from casadi.tools import *
from casadi.tools.graph import *

theta = ssym('theta')

# Straightforward implementation
R1 = SXMatrix(2,2)
R1[0,0] = cos(theta)
R1[0,1] = sin(theta)
R1[1,0] = -sin(theta)
R1[1,1] = cos(theta)
dotdraw(R1)
f1 = SXFunction([theta],[R1])
f1.init()
f1.generateCode('R1.c')

# Manually avoiding common subexpressions
R2 = SXMatrix(2,2)
sintheta = sin(theta)
costheta = cos(theta)
R2[0,0] =  costheta
R2[0,1] =  sintheta
R2[1,0] = -sintheta
R2[1,1] =  costheta
dotdraw(R2)
f2 = SXFunction([theta],[R2])
f2.init()
f2.generateCode('R2.c')

# Try R1^n and R2^n
RList = [R1, R2]
sList = ['R1', 'R2']
for i in xrange(len(sList)):
	R = RList[i]
	s = sList[i]
	for x in xrange(3):
		R = mul(R, RList[i])
		s+=sList[i]
		f = SXFunction([theta],[R])
		f.init()
		f.generateCode(s+'.c')

system('call_statistics')
