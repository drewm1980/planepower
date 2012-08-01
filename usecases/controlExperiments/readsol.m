% #!/usr/bin/octave 

close all
X = dlmread('dataClean.dat');
X = X(1:end-3,:);
t = X(:,1);
x = X(:,2);
y = X(:,3);
z = X(:,4);
vx = X(:,5);
vy = X(:,6);
vz = X(:,7);
e11 = X(:,8);
e12 = X(:,9);
e13 = X(:,10);
e21 = X(:,11);
e22 = X(:,12);
e23 = X(:,13);
e31 = X(:,14);
e32 = X(:,15);
e33 = X(:,16);
omegax = X(:,17);
omegay = X(:,18);
omegaz = X(:,19);
delta = X(:,20);
ddelta = X(:,21);
delta_meas = X(:,22);
ddelta_meas = X(:,23);
omegax_meas = X(:,24);
omegay_meas = X(:,25);
omegaz_meas = X(:,26);
ax_meas = X(:,27);
ay_meas = X(:,28);
az_meas = X(:,29);
omegax_pred = X(:,30);
omegay_pred = X(:,31);
omegaz_pred = X(:,32);
ax_pred = X(:,33);
ay_pred = X(:,34);
az_pred = X(:,35);
orth = [];
for i=1:numel(e11)
    R = [e11(i) e12(i) e13(i); e21(i) e22(i) e23(i); e31(i) e32(i) e33(i)];
    F = R*R';
    orth = [orth; F(1,1)];
end
figure(7)
plot3(x,y,z)
axis equal

figure(8)
plot(x,y,'.')
title('x vs y')
figure(9)
subplot(311)
plot(t,x)
ylabel('x')
title('position')
subplot(312)
plot(t,y)
ylabel('y')
subplot(313)
plot(t,z)
ylabel('z')
figure(10)
subplot(311)
plot(t,vx)
subplot(312)
plot(t,vy)
subplot(313)
plot(t,vz)
title('velocity')

figure(11)
subplot(311)
plot(t,ax_meas)
subplot(312)
plot(t,ay_meas)
subplot(313)
plot(t,az_meas)
title('accelerations')
figure(12)
subplot(311)
plot(t,omegax_meas)
subplot(312)
plot(t,omegay_meas)
subplot(313)
plot(t,omegaz_meas)
title('angular velocities')
omega=[];
%%
for i=1:numel(x)
    omega = [omega; sqrt(X(i,17)^2+X(i,18)^2+X(i,19)^2)];
end
a=[];
for i=1:numel(x)
    a = [a; sqrt(X(i,24)^2+X(i,25)^2+X(i,26)^2)];
end
figure(1)
plot(t,omega)
title('norm of omega')

figure(2)
plot(t,a)
title('norm of a')
nn = []
for i=1:numel(x)
    nn=[nn; x(i)^2+y(i)^2+z(i)^2+vx(i)^2];
end
h=0.00125;
r = 1.20;
dr = 0.0;
rA = 1.085;
dddelta = 0.0;
xA = rA*cos(delta); 
dxA = -ddelta.*rA.*sin(delta); 
ddxA = (-dddelta.*rA.*sin(delta) - ddelta.*ddelta.*rA.*cos(delta)); 
yA = rA.*sin(delta); 
dyA = ddelta.*rA.*cos(delta); 
ddyA = -rA.*sin(delta).*ddelta.*ddelta + dddelta.*rA.*cos(delta);
dx=vx;
dy=vy;
dz=vz;
const = - r.*r/2 + x.*x/2 - x.*xA + xA.*xA/2 + y.*y/2 - y.*yA + yA.*yA/2 + z.*z/2;
dConst = dx.*x - dr.*r - dxA.*x - dx.*xA + dxA.*xA + dy.*y - dyA.*y - dy.*yA + dyA.*yA + dz.*z;
figure(3)
subplot(211)
plot(t,const)
title('consistency condition')
subplot(212)
plot(t,dConst)
%%
figure(4)
subplot(611)
plot(t,omegax_meas,'r')
hold on
plot(t,omegax_pred)
subplot(612)
plot(t,omegay_meas,'r')
hold on
plot(t,omegay_pred)
subplot(613)
plot(t,omegaz_meas,'r')
hold on
plot(t,omegaz_pred)
subplot(614)
plot(t,ax_meas,'r')
hold on
plot(t,ax_pred)
subplot(615)
plot(t,ay_meas,'r')
hold on
plot(t,ay_pred)
subplot(616)
plot(t,az_meas,'r')
hold on
plot(t,az_pred)


%%



% error_tether_length = ((x-1.085*cos(delta)).^2+(y-1.085*sin(delta)).^2+z.^2).^0.5 - tether_length;
% figure
% plot(t,error_tether_length)
% title('error in tether length [m]')
% dddelta = 0.0;
% rA = tether_length;
% xA = -rA*sin(delta); 
% dxA = -ddelta.*rA.*cos(delta); 
% ddxA = -(dddelta.*rA.*cos(delta) - ddelta.*ddelta.*rA.*sin(delta)); 
% yA = rA.*cos(delta); 
% dyA = -ddelta.*rA.*sin(delta); 
% ddyA = - rA.*cos(delta).*ddelta.*ddelta - dddelta.*rA.*sin(delta); 
% 
%     Const = - r.*r/2 + x.*x/2 - x.*xA + xA*xA/2 + y*y/2 - y*yA + yA*yA/2 + z*z/2; 
% dConst = vx.*x - dr.*r - dxA.*x - vx.*xA + dxA.*xA + vy.*y - dyA.*y - vy.*yA + dyA.*yA + vz.*z
%%

figure(5)
subplot(311)
plot(t,e11,'.')
subplot(312)
plot(t,e21,'.')
subplot(313)
plot(t,e31,'.')

figure(6)
subplot(311)
plot(t,e12,'.')
subplot(312)
plot(t,e22,'.')
subplot(313)
plot(t,e32,'.')

figure(7)
subplot(311)
plot(t,e13,'.')
subplot(312)
plot(t,e23,'.')
subplot(313)
plot(t,e33,'.')
% %%
% 
% camData = dlmread('camData.dat');
% t = camData(:,1);
% figure
% subplot(611)
% plot(t,camData(:,2),'.')
% subplot(612)
% plot(t,camData(:,3),'.')
% subplot(613)
% plot(t,camData(:,4),'.')
% subplot(614)
% plot(t,camData(:,5),'.')
% subplot(615)
% plot(t,camData(:,6),'.')
% subplot(616)
% plot(t,camData(:,7),'.')
% 
% figure
% subplot(611)
% plot(t,camData(:,8),'.')
% subplot(612)
% plot(t,camData(:,9),'.')
% subplot(613)
% plot(t,camData(:,10),'.')
% subplot(614)
% plot(t,camData(:,11),'.')
% subplot(615)
% plot(t,camData(:,12),'.')
% subplot(616)
% plot(t,camData(:,13),'.')
% 
% %%
% nu = dlmread('inovation.dat');
% t = nu(:,1);
% figure
% subplot(211)
% plot(t,nu(:,2),'.')
% subplot(212)
% plot(t,nu(:,3),'.')
