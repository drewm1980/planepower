function [ H,F1,F2,F3,G,E ] = mpc_condensing( A,B,C,D,nPH,nCH,Q,R,P )
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
%


% determine dimensions
nX = size(A,1);
nY = size(C,1);
nU = size(B,2);


Q_aug = Q;
for i=2:nPH-1
    Q_aug = blkdiag(Q_aug,Q);
end
if ( size(P,1) == nX )
    Q_aug = blkdiag(Q_aug,P);
else
    Q_aug = blkdiag(Q_aug,Q);
end

R_aug = R;
for i=2:nCH
    R_aug = blkdiag(R_aug,R);
end

%% E Matrix
CAProd = C*A;               % | CA      |
E = CAProd;                 % | CA^2    |
for i=2:nPH                 % | CA^3    |
    CAProd = CAProd*A;      % | ...     |
    E = vertcat(E,CAProd);  % | CA^nPH |
end


%% calculate constraint matrix G
G = zeros(nY*nPH,nU*nCH);   % | CB
ABProd = B;                 % | CB+CAB          CB
sum = C*B;                  % | CB+CAB+CA^2B    CB+CAB      CB
% main diagonal elements    % | ...
for i=1:nCH                 % | ...
    rs = 1+(nY*(i-1));      % | ...
    re = nY*i;              % | CB+ ... + CA^(nPH-1)B
    cs = 1+(nU*(i-1));      % |
    ce = nU*i;              % |
    G(rs:re,cs:ce) = sum;   % |
end                         % |
% i-th subdiagonal
for i=2:nPH
    ABProd = A*ABProd;
%    sum = sum + C*ABProd;
    for j=i:min(nPH,i+nCH-1)
        rs = 1+(nY*(j-1));  % row start
        re = nY*j;          % row end
        cs = 1+(nU*(j-i));  % column start
        ce = nU*(j-i+1);    % column end
        G(rs:re,cs:ce) = sum+C*ABProd; % sum vorher added sum in here (hammad)
    end
end

% first upper diagonal
for i=2:nCH
    rs = 1+(nY*(i-2));
    re = nY*(i-1);
    cs = 1+(nU*(i-1));
    ce = nU*i;
    G(rs:re,cs:ce) = D;
end


%% calculate Hessian matrix H and gradient matrices
H  = G'*Q_aug*G + R_aug;
H  = 0.5*(H+H'); % symmetrize
F1 = G'*Q_aug'*E;
F2 = G'*Q_aug';
F3 = R_aug';

