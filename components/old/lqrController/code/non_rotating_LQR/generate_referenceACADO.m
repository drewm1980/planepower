function [Tref,Xref,Uref] = generate_referenceACADO(z,r,delta,ddelta,Ts,Nref)




delta0 = delta;
delta = 0;

rA = 1.085;
r1 = rA + sqrt(r^2 - z^2);
x = -r1*sin(delta);
y =  r1*cos(delta);

dx = -r1*cos(delta)*ddelta; 
dy = -r1*sin(delta)*ddelta; 
dz = 0;

%    % Hang aircraft upside-down
TetherAngle = asin(z/r);
psi = pi; % z-axis
the = 0;  % y-axis
phi = -(-pi/2-TetherAngle*pi/180); % -pi/2-TetherAngle*pi/180; % x-axis

q0 =   cos(0.5*psi)*cos(0.5*the)*cos(0.5*phi)  +  sin(0.5*psi)*sin(0.5*the)*sin(0.5*phi);
q1 =   cos(0.5*psi)*cos(0.5*the)*sin(0.5*phi)  -  sin(0.5*psi)*sin(0.5*the)*cos(0.5*phi);
q2 =   cos(0.5*psi)*sin(0.5*the)*cos(0.5*phi)  +  sin(0.5*psi)*cos(0.5*the)*sin(0.5*phi);
q3 = - cos(0.5*psi)*sin(0.5*the)*sin(0.5*phi)  +  sin(0.5*psi)*cos(0.5*the)*cos(0.5*phi);

Q0=q0; Q1=q1; Q2=q2; Q3=q3;

p0 = cos(delta/2);
p1 = 0.;
p2 = 0.;
p3 = sin(delta/2);

q0 = p0*Q0 - p1*Q1 - p2*Q2 - p3*Q3;
q1 = Q1*p0 + Q0*p1 - Q2*p3 + Q3*p2;
q2 = Q2*p0 + Q0*p2 - Q3*p1 + Q1*p3;
q3 = Q3*p0 + Q0*p3 - Q1*p2 + Q2*p1;

E = [[-q1,  q0, -q3,  q2];
     [-q2,  q3,  q0, -q1];           
     [-q3, -q2,  q1,  q0]];

G = [[-q1,  q0,  q3, -q2];
     [-q2, -q3,  q0,  q1];
     [-q3,  q2, -q1,  q0]];

R = E*G.';

% Rotational speed in aricraft's referential
wE = R.'*[0;0;ddelta];
w1 = wE(1);
w2 = wE(2);
w3 = wE(3);

dr = 0;

ur = 0;
up = 0;

X0 = [x;y;z;dx;dy;dz;R(1,:).';R(2,:).';R(3,:).';w1;w2;w3;r;dr;delta;ddelta;ur;up].';
U0 = zeros(1,4);
X0U0 = [0 X0 U0];


!rm eq/EQ_init.txt
save('eq/EQ_init.txt','X0U0','-ascii')

cd eq
!make equilibrium
% !./equilibrium
eval(['!./equilibrium ',num2str(z),'     ',num2str(r),'     ',num2str(delta),'     ',num2str(ddelta)]);
cd ..

formEQ = '[';
for k = 1:size(X0U0,2)
    formEQ = [formEQ,' %f'];
end
formEQ = [formEQ,' ]\n'];
File = 'eq/EQ_params.txt';
fid = fopen(File);
Out = textscan(fid,formEQ);
fclose(fid);


EQLabels = {'t', 'x', 'y', 'z', 'dx', 'dy', 'dz', 'e11', 'e12', 'e13', 'e21', 'e22', 'e23', 'e31', 'e32', 'e33', 'w1', 'w2', 'w3', 'r', 'dr', 'delta', 'ddelta','ur','up', 'dddelta',' ddr', 'dur', 'dup'};

for k = 1:size(Out,2)
    eval([EQLabels{k},' = Out{k};']);
end

e1 = [e11,e12,e13];
e2 = [e21,e22,e23];
e3 = [e31,e32,e33];
R = [e1;e2;e3];

delta = delta0;
Xref = zeros(Nref,24);
Uref = zeros(Nref,4);
Tref = zeros(Nref,1);
for k = 1:Nref
    rot = delta;
        
    p0 = cos(rot/2);
    p1 = 0.;
    p2 = 0.;
    p3 = sin(rot/2);

    E = [[-p1,  p0, -p3,  p2];
         [-p2,  p3,  p0, -p1];           
         [-p3, -p2,  p1,  p0]];

    G = [[-p1,  p0,  p3, -p2];
         [-p2, -p3,  p0,  p1];
         [-p3,  p2, -p1,  p0]];
 
    R2 = E*G.';
    
    Xk = R2*[x;y;z];
    dXk = R2*[dx;dy;dz];

    RR = R2*R;
    
    X0k = [Xk;dXk;RR(1,:).';RR(2,:).';RR(3,:).';w1;w2;w3;r;dr;delta;ddelta;ur;up].';
    
    
    Xref(k,:) = X0k;
    Uref(k,:) = U0;
    Tref(k) = Ts*(k-1);
    
    delta = delta + ddelta*Ts;
end


% Sampling time
% Ts = 0.1;
% Nref = round(Tf/Ts);



% !rm eq/eqIG.txt
% fid = fopen(['eq/eqIG.txt'], 'w');
% IG = [z r delta ddelta Ts Nref];
% fprintf(fid,'%6.16e %6.16e %6.16e %6.16e %6.16e %6.16e \n',IG');
% fclose(fid);
% 
% !/usr/bin/env -i python eq/generate_reference.py
% 
% 
% Xref = load('refXD.txt');
% Uref = load('refU.txt');
% Tref = load('refT.txt');
% 
% Xref = [Xref Uref(:,3:4)];
% Uref(:,3:4) = zeros(size(Uref,1),2);

% 
% for k = 1:Nref
%     rot = delta;
%         
%     p0 = cos(rot/2);
%     p1 = 0.;
%     p2 = 0;
%     p3 = sin(rot/2);
% 
% %     q0 = p0*Q0 - p1*Q1 - p2*Q2 - p3*Q3;
% %     q1 = Q1*p0 + Q0*p1 - Q2*p3 + Q3*p2;
% %     q2 = Q2*p0 + Q0*p2 - Q3*p1 + Q1*p3;
% %     q3 = Q3*p0 + Q0*p3 - Q1*p2 + Q2*p1;
%     
%     
%     E = [[-p1,  p0, -p3,  p2];
%          [-p2,  p3,  p0, -p1];           
%          [-p3, -p2,  p1,  p0]];
%     
%     G = [[-p1,  p0,  p3, -p2];
%          [-p2, -p3,  p0,  p1];
%          [-p3,  p2, -p1,  p0]];
%  
%     R2 = E*G.';
%     
%     wE = R2.'*[w1;w2;w3];
%     
%     Xk = R2*[x;y;z];
%     dXk = R2*[dx;dy;dz];
%         
% %     E = [[-q1,  q0, -q3,  q2];
% %          [-q2,  q3,  q0, -q1];           
% %          [-q3, -q2,  q1,  q0]]);
% %     
% %     G = [[-q1,  q0,  q3, -q2];
% %          [-q2, -q3,  q0,  q1];
% %          [-q3,  q2, -q1,  q0]];
% %     
% %     RR = np.dot(E,G.T);
%     RR = R2*R;
%     
%     X0k = [Xk;dXk;RR(1,:).';RR(2,:).';RR(3,:).';[w1;w2;w3;r;dr;delta;ddelta;ur;up]];
%     
%     Xref(k,:) = X0k;
% %     Uref(k,:) = U0k;
%     Tref(k,:) = (k-1)*Ts;
%     
%     delta = delta + ddelta*Ts;
% end    