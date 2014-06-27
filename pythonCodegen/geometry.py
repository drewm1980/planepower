#!/usr/bin/env python
import numpy
import casadi

#Calculate the line segment PaPb that is the shortest route between
#two lines P1P2 and P3P4. Calculate also the values of mua and mub where
#Pa = P1 + mua (P2 - P1)
#Pb = P3 + mub (P4 - P3)
#Return FALSE if no solution exists.
# Numpy implementation
def line_line_intersect_numpy(p1, p2, p3, p4):
	EPS=.00001
	p13 = p1-p3
	p43 = p4-p3
	p21 = p2-p1
	if (abs(p43[0]) < EPS and abs(p43[1]) < EPS and abs(p43[2]) < EPS):
		assert(False) 
		return False
	if (abs(p21[0]) < EPS and abs(p21[1]) < EPS and abs(p21[2]) < EPS):
		assert(False)
		return False
	d1343 = p13[0] * p43[0] + p13[1] * p43[1] + p13[2] * p43[2];
	d4321 = p43[0] * p21[0] + p43[1] * p21[1] + p43[2] * p21[2];
	d1321 = p13[0] * p21[0] + p13[1] * p21[1] + p13[2] * p21[2];
	d4343 = p43[0] * p43[0] + p43[1] * p43[1] + p43[2] * p43[2];
	d2121 = p21[0] * p21[0] + p21[1] * p21[1] + p21[2] * p21[2];
	denom = d2121 * d4343 - d4321 * d4321;
	if (abs(denom) < EPS):
		assert(False)
		return False
	numer = d1343 * d4321 - d1321 * d4343;
	mua = numer / denom;
	mub = (d1343 + d4321 * mua) / d4343;
	pa = p1 + mua * p21;
	pb = p3 + mub * p43;
	return  pa, pb, mua, mub
# Casadi implementation
def line_line_intersect_casadi(p1,p2,p3,p4):
	p13 = p1-p3
	p43 = p4-p3
	p21 = p2-p1
	d1343 = p13[0,0] * p43[0,0] + p13[1,0] * p43[1,0] + p13[2,0] * p43[2,0];
	d4321 = p43[0,0] * p21[0,0] + p43[1,0] * p21[1,0] + p43[2,0] * p21[2,0];
	d1321 = p13[0,0] * p21[0,0] + p13[1,0] * p21[1,0] + p13[2,0] * p21[2,0];
	d4343 = p43[0,0] * p43[0,0] + p43[1,0] * p43[1,0] + p43[2,0] * p43[2,0];
	d2121 = p21[0,0] * p21[0,0] + p21[1,0] * p21[1,0] + p21[2,0] * p21[2,0];
	denom = d2121 * d4343 - d4321 * d4321;
	numer = d1343 * d4321 - d1321 * d4343;
	mua = numer / denom;
	mub = (d1343 + d4321 * mua) / d4343;
	pa = p1 + mua * p21;
	pb = p3 + mub * p43;
	return  pa, pb, mua, mub
# Overloaded version
def line_line_intersect(p1, p2, p3, p4):
	args = [p1,p2,p3,p4]
	if casadi.SXMatrix in map(type,args):
		assert all([type(arg)==casadi.SXMatrix for arg in args])
		return line_line_intersect_casadi(*args)
	else:
		return line_line_intersect_numpy(*args)

