#!/usr/bin/env python
import numpy
# A small collection of functions for uniformly sampling
# from spaces with weird topology.

# Compute special orthogonal matrices that are in some sense uniform.
# If you take any point and transform it by the matrix from this function,
# it will lie on the hypersphere with the same magnitude, and it will be
# equally likely to be anywhere on the surface of this hypersphere.
def rand_special_orthogonal(n):
	from numpy.linalg import norm
	X = numpy.zeros((n,n))
	x = numpy.random.randn(n)
	x = x/norm(x)
	X[:,0] = x  # Generate a random first column.
	for m in range(1,n): # For every other column of the matrix,
		x = numpy.random.randn(n) # generate a unit vector in a random direction.
		x = x/norm(x)
		for r in xrange(m):
			old = X[:,r]
			# Subtract off the projection onto the other basis vectors
			x = x - numpy.dot(x,old)*old/norm(old)  
			x = x/norm(x)
		X[:,m] = x
	# If the determinant of x is negative, flip the sign of a uniformly
	# randomly chosen basis vector.
	if numpy.linalg.det(X)<0:
		l = numpy.floor(numpy.random.rand()*n)
		X[:,l] = -1*X[:,l]
	return X
