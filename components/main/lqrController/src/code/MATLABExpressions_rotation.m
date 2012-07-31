
 %//////////////////////////////////////////////////////////////////////// 
 %//                                                                    // 
 %//  AUTO-GENERATED EQUATIONS (S. Gros, HIGHWIND, OPTEC, KU Leuven)    // 
 %//                                                                    // 
 %//////////////////////////////////////////////////////////////////////// 

%// Equations read: 
%// IMA = inv(MA) 
%// ddX = IMA*(Bx - CA*lambda) 
%// lambdaNum = CA^T*IMA*Bx - Blambda 
%// lambdaDen = CA^T*IMA*CA 
%// lambda = lambdaNum/lambdaDen 

%// Arm 

xA = -rA*sin(delta); 
dxA = -ddelta*rA*cos(delta); 
ddxA = -(dddelta*rA*cos(delta) - ddelta*ddelta*rA*sin(delta)); 
yA = rA*cos(delta); 
dyA = -ddelta*rA*sin(delta); 
ddyA = - rA*cos(delta)*ddelta*ddelta - dddelta*rA*sin(delta); 
% xA = rA*sin(delta); 
% dxA = ddelta*rA*cos(delta); 
% ddxA = dddelta*rA*cos(delta) - ddelta*ddelta*rA*sin(delta); 
% yA = rA*cos(delta); 
% dyA = -ddelta*rA*sin(delta); 
% ddyA = - rA*cos(delta)*ddelta*ddelta - dddelta*rA*sin(delta); 

%// BUILD DYNAMICS 
lambdaNum = ddxA*xA - 2*dy*dyA - ddr*r - ddxA*x - 2*dx*dxA - ddyA*y + ddyA*yA - dr*dr + dx*dx + dxA*dxA + dy*dy + dyA*dyA + dz*dz + (F(1)*(x - xA))/m + (F(2)*(y - yA))/m + (z*(F(3) - g*mgrav))/m; 

lambdaDen = x*x/m + xA*xA/m + y*y/m + yA*yA/m + z*z/m - (2*x*xA)/m - (2*y*yA)/m; 
%// BUILD CONSTRAINTS 
Const = - r*r/2 + x*x/2 - x*xA + xA*xA/2 + y*y/2 - y*yA + yA*yA/2 + z*z/2; 
dConst = dx*x - dr*r - dxA*x - dx*xA + dxA*xA + dy*y - dyA*y - dy*yA + dyA*yA + dz*z; 

Pole = 1;
lambda = 2*Pole*dConst - Pole*Pole*Const + lambdaNum/lambdaDen; 	

ddX(1,1) = (F(1) - lambda*(x - xA))/m; 
ddX(2,1) = (F(2) - lambda*(y - yA))/m; 
ddX(3,1) = -(g*mgrav - F(3) + lambda*z)/m; 
ddX(4,1) = (I31*(T(3) + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3) - (I3*(T(1) - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3); 
ddX(5,1) = (T(2) + w1*(I31*w1 + I3*w3) - w3*(I1*w1 + I31*w3))/I2; 
ddX(6,1) = (I31*(T(1) - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3) - (I1*(T(3) + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3); 




%// AIRCRAFT REF. FRAME ACCELERATION 
% ddxIMU = ddX(1,1)*(q0*q0 + q1*q1 - q2*q2 - q3*q3) - (ddX(3,1) + g)*(2*q0*q2 - 2*q1*q3) + ddX(2,1)*(2*q0*q3 + 2*q1*q2); 
% ddyIMU = ddX(2,1)*(q0*q0 - q1*q1 + q2*q2 - q3*q3) + (ddX(3,1) + g)*(2*q0*q1 + 2*q2*q3) - ddX(1,1)*(2*q0*q3 - 2*q1*q2); 
% ddzIMU = ddX(1,1)*(2*q0*q2 + 2*q1*q3) - ddX(2,1)*(2*q0*q1 - 2*q2*q3) + (ddX(3,1) + g)*(q0*q0 - q1*q1 - q2*q2 + q3*q3); 
% w1IMU = w1; 
% w2IMU = w2; 
% w3IMU = w3; 

%///////////////////////////// END OF AUTO-GENERATED CODE ////////////////////////////////////////////////////// 
