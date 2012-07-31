function [A,B] = linearize_AC(t,x,u)
u=P.tu;

dx    = Model_integ_ACADO(0,x,'',P);
olddx = dx; 
oldu  = u; 
oldx  = x;

xndx = 1:numStates;
undx = 1:numctrl;
% A matrices
% ----------------

for i=1:length(xndx);
	j      = xndx(i);
	x(j)   = x(j) + xpert(j);
	dx     = Model_integ_ACADO(0,x,'',P);
 	A(:,i) = (dx(xndx) - olddx(xndx))./xpert(j);
	x  = oldx;
end

% B and D matrices
% ----------------

for i=1:length(undx),
	j    = undx(i);
	u(j) = u(j)+upert(j);
	y    = eval(fun2);
	dx   = eval(fun1);
	if ~isempty(B),
	    B(:,i) = (dx(xndx)-olddx(xndx))./upert(j);
	end
	if nylin > 0
		D(:,i) = (y(yndx)-oldy(yndx))./upert(j);
	end
	u = oldu;
end


% para(3) is set to 1 to remove extra states from blocks that are not in the
% input/output path. This removes a class of uncontrollable and unobservable
% states but does not remove states caused by pole/zero cancellation.

if para(3) == 1 
   [A,B,C] = minlin(A,B,C);
end


