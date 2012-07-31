#!/usr/bin/env python
import casadi
import numpy

# Using these results in (very slightly) simpler expressions.
# i.e. compare SXMatrix(1./2)*2 or 1./2*casadi.SXMatrix(2) to half*2
two = casadi.SXMatrix([2])
onehalf = 1/two 

# Take a casadi symbolic expression that is fully specified,
# and actually perform the internal computations.
# same as simplify? or maybe evaluateConstants?
def flatten_sx(S):
	f = casadi.SXFunction([],[S])
	f.init()
	f.evaluate()
	return f.output()

# Take 1x1 Matrix<SX>, evaluate it numerically, return double.
def sx_to_double(sx1,variables,values):
	val1 = casadi.substitute(sx1,variables,values)
	val1 = casadi.getValue(val1)
	import numpy
	if numpy.isnan(val1):
		print 'sx_to_double:Warning: casadi.getValue returned nan! \
Make sure "sx1" is fully specified once \
"values" have been substituted for "variables"'
	return val1

def test_numerically_the_same(sx1,sx2,
							  variables=[],values=[],
							  eps=1e-6, printWhyNot=True):
	if type(sx1) != type(sx2):
		if printWhyNot:
			print "Input expressions not the same type!"
		return False
	if type(variables)==list and variables==[]:
		variables1 = casadi.getFree(sx1)
		variables = variables1
		#variables2 = casadi.getFree(sx2)
		#sanityCheck = test_numerically_the_same(variables1,
											#variables2,
											#variables1)
		#if not sanityCheck:
			#if printWhyNot:
				#print "The two expressions have different arguments!"
			#return False
	if type(values)==list and values==[]:
		import numpy
		values = numpy.random.randn(variables.shape[0],variables.shape[1])
	if variables.shape!=values.shape:
		if printWhyNot:
			print "variables and values should be same shape!"
		return False
	if type(sx1)==casadi.SXMatrix:
		if sx1.shape!=sx2.shape:
			if printWhyNot:
				print "Arrays not the same shape!"
			return False
		h,w = sx1.shape
		passed = True
		for i in xrange(h):
			for j in xrange(w): 
				p = test_numerically_the_same(casadi.SX(sx1[i,j]),
								  			  casadi.SX(sx2[i,j]))
				passed = passed and p
				if not p:
					if printWhyNot:
						print "Element [%i,%i] is not equal!"%(i,j)
		return passed
	elif type(sx1)==casadi.SX:
		val1 = sx_to_double(sx1,variables,values)
		val2 = sx_to_double(sx2,variables,values)
		diff = abs(val1-val2)
		passed = diff<eps
		if not passed and printWhyNot:
			print "Numerical difference %f is larger than eps=%f!"%(diff,eps)
		return passed
	else:
		assert False,"Unhandled type"

def sum_squares(a):
	if type(a)==casadi.SXMatrix:
		return casadi.sumAll(a*a)
	else:
		return (a*a).reshape(-1).sum()

## Compute the jacobian of an SXExpression
## Note that the variables go after the function
## like pretty much every other CAS.
#def jacobian(expression, variables):
	#assert type(expression)==casadi.SXMatrix
	#f = casadi.SXFunction([variables], [expression])
	#f.init();
	#J = f.jacobian().outputSX()
	#return J
# Note:  The above convenience function turned out to already be present in casadi.

# Convenience function for taking the total time derivative of
# a matrix expression.
# Inputs: 
#	M: SXMatrix of expressions that contains only variables,
#				and other expressions that don't depend on time
#	q:  SXMatrix of time-dependent variables
#	dq: SXMatrix of symbols representing the time derivatives
#				of the variables
def total_derivative(M,q,dq):
	J = casadi.jacobian(M, q)
	dM = casadi.mul(J,dq).reshape(M.shape)
	return dM
	
