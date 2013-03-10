#/usr/bin/env python
import numpy
import casadi
from casadi import SXFunction, IdasIntegrator

from carousel_model import constants, dynamic_equations, inputs, t, q, dq, ddq, q_and_dq, dq_and_ddq

# Measure how fast we can simulate the system with an integrator
ppp = casadi.ssym('ppp')
f = SXFunction([t, q_and_dq_and_ddq, ppp, dq_and_ddq], [dynamic_equations])
f.init()
integrator = IdasIntegrator(f)


