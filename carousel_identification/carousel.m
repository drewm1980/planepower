close all
clear all
R = 1;

beta = 30*pi/180;
t = 0:0.01:1;
omega = 2*pi;

delta = omega*t;

alpha = 90*pi/180;

xc = R*cos(delta);
yc = R*sin(delta);
zc = zeros(size(delta));

Rbeta = [cos(beta) 0 sin(beta);
         0 1 0;
         -sin(beta) 0 cos(beta)];
X1 = Rbeta*[xc;yc;zc];
x1 = X1(1,:);
y1 = X1(2,:);
z1 = X1(3,:);

Ralpha = [cos(alpha) -sin(alpha) 0;
          sin(alpha) cos(alpha) 0;
          0 0 1];

X = Ralpha*[x1;y1;z1];

x = X(1,:);
y = X(2,:);
z = X(3,:);

figure
plot3(x,y,z)
xlabel('x')
ylabel('y')
zlabel('z')
rIMU = 0;
pIMU = 0;
yIMU = 0;
aIMU =[- (R.*omega.^2.*sin(omega.*t).*cos(alpha) + R.*omega.^2.*cos(omega.*t).*cos(beta).*sin(alpha)).*(sin(omega.*t).*cos(alpha) + cos(omega.*t).*cos(beta).*sin(alpha)) - (R.*omega.^2.*sin(omega.*t).*sin(alpha) - R.*omega.^2.*cos(omega.*t).*cos(alpha).*cos(beta)).*(sin(omega.*t).*sin(alpha) - cos(omega.*t).*cos(alpha).*cos(beta)) - R.*omega.^2.*cos(omega.*t).^2.*sin(beta).^2;
 R.*omega.^2.*cos(omega.*t).*sin(omega.*t).*sin(beta).^2 - (R.*omega.^2.*sin(omega.*t).*sin(alpha) - R.*omega.^2.*cos(omega.*t).*cos(alpha).*cos(beta)).*(cos(omega.*t).*sin(alpha) + sin(omega.*t).*cos(alpha).*cos(beta)) - (R.*omega.^2.*sin(omega.*t).*cos(alpha) + R.*omega.^2.*cos(omega.*t).*cos(beta).*sin(alpha)).*(cos(omega.*t).*cos(alpha) - sin(omega.*t).*cos(beta).*sin(alpha));
                                                                                        sin(alpha).*sin(beta).*(R.*omega.^2.*sin(omega.*t).*cos(alpha) + R.*omega.^2.*cos(omega.*t).*cos(beta).*sin(alpha)) - cos(alpha).*sin(beta).*(R.*omega.^2.*sin(omega.*t).*sin(alpha) - R.*omega.^2.*cos(omega.*t).*cos(alpha).*cos(beta)) - R.*omega.^2.*cos(omega.*t).*cos(beta).*sin(beta)];
																																												 

figure
subplot(311)
plot(aIMU(1,:))
subplot(312)
plot(aIMU(2,:))
subplot(313)
plot(aIMU(3,:))

%%
clear all
syms alpha beta R omega t real
syms delt(t) real

delta = delt+alpha;
Rdelta = [cos(delta) -sin(delta) 0;
          sin(delta) cos(delta) 0;
          0 0 1];

Xc = Rdelta*[R;0;0];

Rbeta = [cos(beta) 0 sin(beta);
         0 1 0;
         -sin(beta) 0 cos(beta)];

X = Rbeta*Xc;

a = diff(diff(X,t),t);


syms rIMU pIMU yIMU real;
Ry = [cos(yIMU) -sin(yIMU) 0;
      sin(yIMU) cos(yIMU) 0;
      0 0 1];
Rp = [cos(pIMU) 0 sin(pIMU);
      0 1 0;
      -sin(pIMU) 0 cos(pIMU)];
Rr = [1 0 0;
      0 cos(rIMU) -sin(rIMU);
      0 sin(rIMU) cos(rIMU)];
syms omega real;
Rimu = Ry*Rp*Rr;
wx = 0;
wy = 0;
wz = omega;

w = Rbeta*Rdelta*[wx;wy;wz];
syms g real;
G = [0;0;g];
aIMU = Rdelta.'*Rbeta'*(a+G);
wIMU = Rdelta.'*Rbeta'*w;

aIMU = simplify(aIMU);
wIMU = simplify(wIMU);

aIMU = Rimu*aIMU;
wIMU = Rimu*wIMU;


%%
%Create some data
clear all
alpha = 10*pi/180;
beta = 1*pi/180;
t = 0:1/800:10;
omega = 2*pi;
g = 9.81;
R = 1;

rIMU = 0.5;
pIMU = 0.5;
yIMU = 0.5;

delta_t = omega*t;

aIMU = zeros(numel(delta_t),3);
wIMU = zeros(numel(delta_t),3);

for i=1:numel(delta_t)
    delta = delta_t(i);
    aIMU(i,1) = g*cos(beta)*(sin(rIMU)*sin(yIMU) + cos(rIMU)*cos(yIMU)*sin(pIMU)) - cos(pIMU)*cos(yIMU)*(R*omega^2 + g*cos(alpha + delta)*sin(beta)) - g*sin(alpha + delta)*sin(beta)*(cos(rIMU)*sin(yIMU) - cos(yIMU)*sin(pIMU)*sin(rIMU));
    aIMU(i,2) = g*sin(alpha + delta)*sin(beta)*(cos(rIMU)*cos(yIMU) + sin(pIMU)*sin(rIMU)*sin(yIMU)) - g*cos(beta)*(cos(yIMU)*sin(rIMU) - cos(rIMU)*sin(pIMU)*sin(yIMU)) - cos(pIMU)*sin(yIMU)*(R*omega^2 + g*cos(alpha + delta)*sin(beta));
    aIMU(i,3) = sin(pIMU)*(R*omega^2 + g*cos(alpha + delta)*sin(beta)) + g*cos(beta)*cos(pIMU)*cos(rIMU) + g*sin(alpha + delta)*cos(pIMU)*sin(beta)*sin(rIMU);
    wIMU(i,1) = omega*(sin(rIMU)*sin(yIMU) + cos(rIMU)*cos(yIMU)*sin(pIMU));
    wIMU(i,2) =  -omega*(cos(yIMU)*sin(rIMU) - cos(rIMU)*sin(pIMU)*sin(yIMU));
    wIMU(i,3) = omega*cos(pIMU)*cos(rIMU);
end

dlmwrite('aIMU.dat',aIMU+0.05*randn(size(aIMU)),'delimiter',' ','precision',10)
dlmwrite('wIMU.dat',wIMU+0.005*randn(size(wIMU)),'delimiter',' ','precision',10)
dlmwrite('delta.dat',delta_t','delimiter',' ','precision',10)

