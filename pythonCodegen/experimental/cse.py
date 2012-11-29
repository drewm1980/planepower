#!/usr/bin/env python

# Functions for performing common subexpression 
# elimination of casadi expressions.

import casadi
from casadi import SX

# Dictionary mapping hash values to git-style object signatures
# Something like a recursive hashmap
casadi_cse_recursive_hashmap = {}

# Dictionary mapping hash values to cse'd casadi expressions
casadi_cse_hashed_expressions = {}

# Dictionary mapping casadi expressions to hash values.
# Essentially the reverse of the above map, and including maps
# for nodes that have not been cse'd.
casadi_cse_expression_hashes = {}

# Some getOp() mappings:

# Binary:
# + 1
# - 2
# * 3
# / 4
binary_opcodes = [1,2,3,4]

# Unary:
# - 5
# sqrt 10
# sin 11
# cos 12
# tan 13
unary_opcodes = [5,10,11,12,13]
all_opcodes = binary_opcodes + unary_opcodes

def apply_unary_opcode(code, p):
	assert code in unary_opcodes, "Opcode not recognized!"
	if code==5:
		return -p
	elif code==10:
		return casadi.sqrt(p)
	elif code==11:
		return casadi.sin(p)
	elif code==12:
		return casadi.cos(p)
	elif code==13:
		return casadi.tan(p)
	assert False

def apply_binary_opcode(code, a, b):
	assert code in binary_opcodes, "Opcode not recognized!"
	if code==1:
		return a+b
	if code==2:
		return a-b
	if code==3:
		return a*b
	if code==4:
		return a/b
	assert False

def isBinaryOperator(node):
	if node.isConstant() or node.isLeaf():
		return False
	assert node.getOp() in all_opcodes, "Opcode not recognized!"
	return node.getOp() in binary_opcodes

def isUnaryOperator(node):
	if node.isConstant() or node.isLeaf():
		return False
	assert node.getOp() in all_opcodes, "Opcode not recognized!"
	return node.getOp() in unary_opcodes

def isLeaf(node):
	return node.isLeaf()

mergeVariablesOfSameName = False

def hash_node(node):
	if casadi_cse_expression_hashes.has_key(node):
		return casadi_cse_expression_hashes[node] 
	if isBinaryOperator(node):
		child0hash = hash_node(node.getDep(0))
		child1hash = hash_node(node.getDep(1))
		if node.isCommutative() and child1hash<child0hash:
			child0hash,child1hash=child1hash,child0hash
		mysig = (node.getOp(), child0hash, child1hash)
		myhash = hash(mysig)
		casadi_cse_recursive_hashmap[myhash] = mysig
		casadi_cse_expression_hashes[node] = myhash
		return myhash
	if isUnaryOperator(node):
		childhash = hash_node(node.getDep(0))
		mysig = (node.getOp(), childhash)
		myhash = hash(mysig)
		casadi_cse_recursive_hashmap[myhash] = mysig
		casadi_cse_expression_hashes[node] = myhash
		return myhash
	if node.isConstant():
		mysig = (node.getValue(),)
		myhash = hash(mysig)
		casadi_cse_recursive_hashmap[myhash] = mysig
		casadi_cse_expression_hashes[node] = myhash
		return myhash
	if node.isSymbolic(): 
		global mergeVariablesOfSameName
		if mergeVariablesOfSameName:
			# Merge variables BY STRING NAME. 
			# Makes the hashmap pickleable, but forces user to use
			# unique names for all expressions.
			mysig = (node.getName(),) 
		else:
			# Merge variables BY SX POINTER.
			# Allows the user to have unnamed,
			# or redundantly-named symbolics in an expression.
			mysig = (node,) 
		myhash = hash(mysig)
		casadi_cse_recursive_hashmap[myhash] = mysig
		casadi_cse_expression_hashes[node] = myhash
		return myhash
	assert False, "Ran into an unhandled node type!"

# Take a the hash value for an (un-cse'd) casadi node,
# and return an equivalent cse'd casadi node.
def hash_to_casadi_node(h):
	myhash = h
	# Node already has an associated cse'd casadi node
	if casadi_cse_hashed_expressions.has_key(myhash):
		return casadi_cse_hashed_expressions[myhash]
	mysig = casadi_cse_recursive_hashmap[myhash]
	# Node is a leaf
	if len(mysig)==1:
		node = casadi.SX(mysig[0])
		casadi_cse_hashed_expressions[myhash] = node
		return node
	# Node is a unary operator
	if len(mysig)==2:
		opcode = mysig[0]
		childnode = hash_to_casadi_node(mysig[1])
		node = apply_unary_opcode(opcode, childnode)
		casadi_cse_hashed_expressions[myhash] = node
		return node
	# Node is a binary operator
	if len(mysig)==3:
		opcode = mysig[0]
		childnode0 = hash_to_casadi_node(mysig[1])
		childnode1 = hash_to_casadi_node(mysig[2])
		node = apply_binary_opcode(opcode, childnode0, childnode1)
		casadi_cse_hashed_expressions[myhash] = node
		return node

# Hashing CSE using recursion
#
# Input:
#	casadi expression
# Output:
#	equivalent casadi expression, hopefully with fewer nodes.
def cse(node):
	if type(node)==casadi.SX:
		h = hash_node(node)
		return hash_to_casadi_node(h)
	if type(node)==casadi.SXMatrix:
		M = node
		shape = M.shape
		l = list(M) # Column-major conversion of M to a python list
		hashes = map(hash_node,l)
		nodes = map(hash_to_casadi_node, hashes)
		l_casadi = casadi.SXMatrix(nodes)
		return l_casadi.reshape(shape)

if __name__=='__main__':
	pass

