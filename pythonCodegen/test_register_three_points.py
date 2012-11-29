#!/usr/bin/env python
from register_three_points import *
from rotation_matrices import pack_se3, append_one

if 0:
	from rand_weird_topology import rand_special_orthogonal
	R1 = rand_special_orthogonal(3)
	t1 = numpy.random.randn(3)
	a1 = numpy.random.randn(3)
	a2 = numpy.random.randn(3)
	a3 = numpy.random.randn(3)
else:
	A = numpy.array([[-0.070498,  -1.444182,   0.299187],
						[0.363679,  -0.393525,  -0.495965],
						[-2.951591,  -1.850025,  -2.753488]])
	a1 = A[:,0]
	a2 = A[:,1]
	a3 = A[:,2]
	R1 = numpy.array([[-0.995384,  -0.094310,  -0.017766],
					[0.082764,  -0.937298,   0.338561],
					[-0.048582,   0.335528,   0.940777]])
	t1 = numpy.array([1.50109, 0.38736, 1.09781])

T1 = pack_se3(R1,t1)
b1 = numpy.dot(T1,append_one(a1))[:3]
b2 = numpy.dot(T1,append_one(a2))[:3]
b3 = numpy.dot(T1,append_one(a3))[:3]

R2,t2 = register_three_points(a1,a2,a3,b1,b2,b3)
T2 = pack_se3(R2,t2)

assert ((T1-T2)<.001).all()

