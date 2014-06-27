clear all
close all
clc


Parameters
Nturn = 2;
RPM = 60;

TetherAngle = 60*10/RPM; %degrees (down from horizontal)
r = 1.18;            %Cable length

P.Tf = 2*Nturn;

ddelta = -2*pi*RPM/60;%4*Nturn*pi/P.Tf;
%return
AZ = 0*pi/180; %Violation of consitency condition if not 0... 
EL = 0*pi/180;

delta = 0;
dddelta = 0;
ArmKinFile

x = XA(1) ;
y = XA(2) + r*cos(TetherAngle*pi/180);
z =  - r*sin(TetherAngle*pi/180);

dx = y*ddelta;
dy = 0;
dz = 0;

%Hang aircraft upside-down
psi = pi; % z-axis
the = 0; % y-axis
phi = -(-pi/2-TetherAngle*pi/180);%-pi/2-TetherAngle*pi/180; % x-axis

q0 =   cos(0.5*psi)*cos(0.5*the)*cos(0.5*phi)  +  sin(0.5*psi)*sin(0.5*the)*sin(0.5*phi);
q1 =   cos(0.5*psi)*cos(0.5*the)*sin(0.5*phi)  -  sin(0.5*psi)*sin(0.5*the)*cos(0.5*phi);
q2 =   cos(0.5*psi)*sin(0.5*the)*cos(0.5*phi)  +  sin(0.5*psi)*cos(0.5*the)*sin(0.5*phi);
q3 = - cos(0.5*psi)*sin(0.5*the)*sin(0.5*phi)  +  sin(0.5*psi)*cos(0.5*the)*cos(0.5*phi);

dr = 0;


 X = [ x; y; z ];
 
  
 E = [-q1  q0 -q3  q2;
      -q2  q3  q0 -q1;           
      -q3 -q2  q1  q0];

 G = [-q1  q0  q3 -q2;
      -q2 -q3  q0  q1;
      -q3  q2 -q1  q0]; 
  
  R = E*G'; 
  
  wE = R'*[0;0;-ddelta];
  w1 = wE(1);
  w2 = wE(2);
  w3 = wE(3);

  
            


state = [x;y;z;dx;dy;dz;q0;q1;q2;q3;w1;w2;w3;r;dr;delta;ddelta];

TP = 1;
tu = [0     0 0 -10 0;
      TP/2  0 0 -10 0];
for k = 1:2*P.Tf/TP-1
    tu = [tu;
          tu(end,1)       0 0 -tu(end,4) 0;
          tu(end,1)+TP/2  0 0 -tu(end,4) 0];
end


P.tu = tu;

figure(33);clf
subplot(2,1,1)
plot(tu(:,1),tu(:,4),'linewidth',2,'color','r');hold on

subplot(2,1,2)
plot(tu(:,1),tu(:,5),'linewidth',2,'color','r');hold on

QuatCheck = q0^2 + q1^2 + q2^2 + q3^2

E = [-q1  q0 -q3  q2;
     -q2  q3  q0 -q1;
     -q3 -q2  q1  q0];
 
G = [-q1  q0  q3 -q2;
     -q2 -q3  q0  q1;
     -q3  q2 -q1  q0];
 
R = E*G';



figure(100);clf
    whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])
 
dddelta = 0;
ArmKinFile

Xc = [x;y;z];

fA = Model_integ_ACADO(0,state,'output',P);


% Scale = 0.1;
% col = {'r','g','c'};
% for k = 1:3
%     line([x x+Scale*R(1,k)],[y y+Scale*R(2,k)],[z z+Scale*R(3,k)],'linewidth',1,'color',col{k});hold on
% end
axis equal
line([XA(1) Xc(1)],[XA(2) Xc(2)],[0 Xc(3)],'linewidth',1,'color','b');hold on
line([0 XA(1)],[0 XA(2)],[0 0],'linewidth',2,'color','b');hold on

plot3(x,y,z,'marker','.','markersize',15,'color','b')
text(x,y,z,'C.M.');%,'Horizontalalignement','left')
plot3(Xc(1),Xc(2),Xc(3),'marker','.','markersize',15,'color','r')
text(Xc(1),Xc(2),Xc(3),'Tether Anchor');%,'Horizontalalignement','left')
% quiver3(x,y,z,dx,dy,dz,'linewidth',1,'color','m');
line([XA(1) x],[XA(2) y],[0 z],'color','k')
E1e = R*[1;0;0];E2e = R*[0;1;0];E3e = R*[0;0;1];
for k = 1:3
    quiver3(x,y,z,R(1,k),R(2,k),R(3,k),0.5,'linewidth',2);hold on
end

% quiver3(x,y,z,fA(1),fA(2),fA(3),5e2,'linewidth',2,'color','k')
% quiver3(x,y,z,fA(4),fA(5),fA(6),5e3,'linewidth',2,'color','k')

xlabel('x')
ylabel('y')
zlabel('z')
view(113,4)
grid

drawnow

X0 = [state];

Out = Model_integ_ACADO(0,X0,'const',P);
ConstCheck = Out(1:2)
% X = [x;y;z];
% Xc = X + R*XTE;

% ((Xc-XA)'*(Xc-XA) - r^2)
% 
% return
%SIMULATE

intoptions.AbsTol = 1e-4;
intoptions.RelTol = 1e-4;
intoptions.MaxStep = 1/10;
intoptions.Events = 'off';


% %Pre-run
% P.tu = 0*tu ;
% [time,state]=ode45('Model_integ_ACADO',[0 5*ddelta/2/pi],X0,intoptions,P);

%[time,state]=ode45('Model_integ_ACADO',[0 P.Tf],X0,intoptions,P);

%X0 = state(end,:);
% return
P.tu = tu ;

tic
[time,state]=ode45('Model_integ_ACADO',[0 P.Tf],X0,intoptions,P);
ET = toc;
RelTime = num2str(P.Tf/ET);
display(['Simulation time ratio: ',RelTime]);

for k = 1:length(time)
    Forces(:,k) = Model_integ_ACADO(time(k),state(k,:),'output',P);
end
Out = [];
for k = 1:length(time)
    Out(:,k) = Model_integ_ACADO(time(k),state(k,:),'const',P);
end

figure(2)
   whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])
labels= {'Const','dConst','lambda','CL' }; 
for k = 1:size(Out,1)
    subplot(2,2,k)
    plot(time,Out(k,:),'linewidth',2);
    title(labels{k})
end

Out = [];
for k = 1:length(time)
    Out = Model_integ_ACADO(time(k),state(k,:),'meas',P);
    IMUMeas(k,:) = Out(1:6);
    ddXE(k,:) = Out(7:end);
end
figure(4)
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])
labels= {'ax','ay','az','w_1','w_2','w_3' }; 
for k = 1:length(labels)
    subplot(2,3,k)
    plot(time,state(:,10+k),'linewidth',2);
    title(labels{k})
end

figure(3)
   whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])
labels= {'w_1','w_2','w_3' }; 
for k = 1:length(labels)
    subplot(2,2,k)
    plot(time,state(:,10+k),'linewidth',2);
    title(labels{k})
end

figure(5)
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])
labels= {'IMU x','IMU y','IMU z','IMU w_1','IMU w_2','IMU w_3' }; 
for k = 1:length(labels)
    subplot(2,3,k)
    plot(time,IMUMeas(:,k),'linewidth',2);hold on
    title(labels{k})
end

for k = 1:3
    subplot(2,3,k)
    plot(time,ddXE(:,k),'linewidth',2,'color','r','linestyle',':');
    title(labels{k})
end
for k = 4:6
    subplot(2,3,k)
    plot(time,state(:,k+7),'linewidth',2,'color','r','linestyle',':');
    title(labels{k})
end
legend('Shift corrected','C.M.')

figure(6)
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])
labels= {'F_x','F_y','F_z','T_x','T_y','T_z' }; 
for k = 1:length(labels)
    subplot(2,3,k)
    plot(time,Forces(k,:),'linewidth',2);hold on
    title(labels{k})
end


%state = [dx;dy;dz;ddx;ddy;ddz;dq0;dq1;dq2;dq3;dw1;dw2;dw3;dr;ddr;ddelta;dddelta]
tx = [time state];
ty = [time IMUMeas state(:,14:17) ];
%tp = [time 2*ones(size(time)) 1.5*ones(size(time)) 0.5*ones(size(time)) 0.0*ones(size(time)) 0*ones(size(time)) 0*ones(size(time))];
tp = [time ones(length(time),1)];% zeros(length(time),1)];

save DataSim tx tp ty tu 
save Cspeed RPM