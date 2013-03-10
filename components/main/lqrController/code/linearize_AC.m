function [A,B] = linearize_AC(t,x,u,xpert,upert,numStates,numCtrl)
P.tu = u;

dx    = Model_integ_ACADO(0,x,'',P);
olddx = dx; 
oldu  = u; 
oldx  = x;
A  = zeros(numStates,numStates);
B = zeros(numStates,numCtrl);
xndx = 1:numStates;
undx = 1:numCtrl;
% A matrix
% ----------------

for i=1:length(xndx);
	j      = xndx(i);
	x(j)   = x(j) + xpert(j);
	dx     = Model_integ_ACADO(t,x,'',P);
 	A(:,i) = (dx(xndx) - olddx(xndx))./xpert(j);
	x  = oldx;
end

% B matrix
% ----------------

for i=1:length(undx),
	j    = undx(i);
	u(j) = u(j)+upert(j);
    P.tu=u;
	dx     = Model_integ_ACADO(t,x,'',P);
	if ~isempty(B),
	    B(:,i) = (dx(xndx)-olddx(xndx))./upert(j);
	end
	u = oldu;
end


