clear all
close all
clc

syms x y z dx dy dz r dr real
syms e11 e12 e13 e21 e22 e23 e31 e32 e33 real
syms delta ddelta real
syms w1 w2 w3 real
% syms ur up real
syms rA real


X = [x;y;z];
dX = [dx;dy;dz];



xA = -rA*sin(delta); 
dxA = -(ddelta*rA*cos(delta)); 
yA = rA*cos(delta); 
dyA = -ddelta*rA*sin(delta); 

XA = [xA;yA;0];
dXA = [dxA;dyA;0];

Rd   = [[cos(-delta), sin(-delta),0];[-sin(-delta), cos(-delta), 0];[0,0,1]];
dRd  = [[-sin(-delta), cos(-delta),0];[-cos(-delta), -sin(-delta), 0];[0,0,0]]*ddelta;
% ddRd  = [[-sin(delta), cos(delta),0];[-cos(delta), -sin(delta), 0];[0,0,0]]*dddelta + [[-cos(delta), -sin(delta),0];[sin(delta), -cos(delta), 0];[0,0,0]]*ddelta*ddelta;
        
% X = Rd.'*Xr;
% dX = Rd.'*( dXr - dRd*X );
% XA = Rd.'*XAr;
% dXA = Rd.'*( dXAr - dRd*XA );

XDr = X-XA;
dXDr = dX-dXA;

% XD = Rd.'*XDr;
% dXD = Rd.'*( dXDr - dRd*XD );
XD = XDr;
dXD = dXDr;

w = [w1;w2;w3];

e1 = [e11,e12,e13];
e2 = [e21,e22,e23];
e3 = [e31,e32,e33];
% R = [e1;e2;e3];
% % R = Rd.'*Rr;
% R = Rr;
% e1 = R(1,:);
% e2 = R(2,:);
% e3 = R(3,:);

Const = - r*r/2 + x*x/2 - x*xA + xA*xA/2 + y*y/2 - y*yA + yA*yA/2 + z*z/2; 
dConst = dx*x - dr*r - dxA*x - dx*xA + dxA*xA + dy*y - dyA*y - dy*yA + dyA*yA + dz*z; 
% ConstR1 = e11*e11 + e12*e12 + e13*e13 - 1;
% ConstR2 = e11*e21 + e12*e22 + e13*e23;
% ConstR3 = e11*e31 + e12*e32 + e13*e33;
% ConstR4 = e21*e21 + e22*e22 + e23*e23 - 1;
% ConstR5 = e21*e31 + e22*e32 + e23*e33;
% ConstR6 = e31*e31 + e32*e32 + e33*e33 - 1;

C = [
%      Const;
%      dConst;

     0.5*(XD.'*XD-r^2);
     XD.'*dXD - r*dr;

     e1*e1.' - 1;
     e1*e2.';
     e1*e3.';
     e2*e1.';
     e2*e2.' - 1;
     e2*e3.';
     e3*e1.';
     e3*e2.';
     e3*e3.' - 1;
     ];
 
 

syms delta1 ddelta1 real

state = [X; dX; e1.'; e2.'; e3.'; w; r; dr; delta1; ddelta1;];
% state = [X; dX; e1.'; e2.'; e3.'; w; r; dr; delta; ddelta;];



J = simplify(jacobian(C,state));

state = [X; dX; e1.'; e2.'; e3.'; w; r; dr; delta; ddelta;];

matlabFunction(J,'file','invariantJac','vars',{state.',rA});