#!/usr/bin/env python

# Functions for performing common subexpression 
# elimination of casadi expressions.

import casadi
import casadi_monkeypatches

from casadi import SX, SXFunction, getFree

from cse import *

def test_single_case(e, verbose=True):
	print "____________________________________"
	from casadi_conveniences import test_numerically_the_same
	cn = casadi.countNodes
	e2 = cse(e)
	passed = test_numerically_the_same(e,e2)
	if passed:
		print "Test passed."
	else:
		print "TEST FAILED!!!!!"
	if verbose or not passed:
		print "Before: "
		print e
		print "After: "
		print e2
		nodes1 = cn(e)
		nodes2 = cn(e2)
		print "Went from %i to %i nodes, i.e. down by %3.1f percent."%(nodes1,nodes2,float(nodes1-nodes2)/nodes1*100)
	return passed

if __name__=='__main__':
	cn = casadi.countNodes
	testExpressions = []
	a = SX('a')
	testExpressions.append(a) # Trivial expression
	b = SX('b')
	c = SX('c')
	c = a*b+b*a-2.0*a # Significant pre-existing subexpression
	testExpressions.append(c)
	e = (a+c)+(a+c)+1.234
	testExpressions.append(e)
	f = (a+b)+(a+b)+35.5
	testExpressions.append(f)
	g = 2*(a+b)*casadi.cos(a+b)*casadi.sin(a+b)
	testExpressions.append(g)
	M = casadi.SXMatrix([[a,e],[f,g]])
	testExpressions.append(M)

	#from carousel_model import ODE_RHS
	#testExpressions.append(ODE_RHS)

	map(test_single_case,testExpressions)
