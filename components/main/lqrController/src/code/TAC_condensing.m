% MPC_CONDENSING: Generates QP matrices from MPC problem by 
% eliminating the differential states from the formulation
%
% function [H,F1,F2,F3,G,E] =
% mpc_condense(A,B,C,D,nPH,nCH,lbU,ubU,lbY,ubY,Q,R,P)
%
% Input:
%   A       system's A matrix
%   B       system's B matrix
%   C       system's C matrix
%   D       system's D matrix
%   nPH     length of prediction horizon
%   nCH     length of control horizon
%   Q       output weight matrix
%   R       control weight matrix
%   P       terminal penalty matrix
%
% Output:
%   H       Hessian matrix
%   F1      gradient-matrix 1 (see below)
%   F2      gradient-matrix 2 (see below)
%   F3      gradient-matrix 3 (see below)
%   G       constraint-matrix 1 (see below)
%   E       constraint matrix 2 (see below)
%
% The following parametric QP is generated
%
%       min   1/2 x'*H*x + x'*(F1*w0 - F2*yRef - F3*uRef)
%        x
%
%       s.t.  lb          <=    x  <=  ub
%             lbG - E*w0  <=  G*x  <=  ubG - E*w0
%
%       where w0, yRef and uRef are given parameters:
%       w0    current state at time k
%       yRef  output reference trajectory for time instants  k+1 to k+nPH
%       uRef  control reference trajectory for time instants k   to k+nCH-1

init_lin_rotation
nPH = 10;
nCH = 8;
lbU= [-3;-3;-3;-3]';
ubU = [3;3;3;3]';
lbY = -1*eye(22,1)';
ubY = 1*eye(22,1)';

[H,F1,F2,F3,G,E] =  mpc_condensing( A,B,C,D,nPH,nCH,Q,R,S );
%