#!/usr/bin/env python
# import this after casadi but ~before any code that needs
#	monkeypatched versions of casadi code.

import casadi

# Workaround bug in casadi countNodes
if casadi.countNodes(casadi.ssym('a'))==2:
	casadi.countNodes_unpatched = casadi.countNodes
	def countNodes_patched(e):
		return casadi.countNodes_unpatched(e)-1
	casadi.countNodes = countNodes_patched

