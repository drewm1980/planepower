XA(1,1) =rA*sin(delta); 
XA(2,1) =rA*cos(delta); 
dXA(1,1) =ddelta*rA*cos(delta); 
dXA(2,1) =-ddelta*rA*sin(delta); 
ddXA(1,1) =dddelta*rA*cos(delta) - ddelta^2*rA*sin(delta); 
ddXA(2,1) =- rA*cos(delta)*ddelta^2 - dddelta*rA*sin(delta); 
