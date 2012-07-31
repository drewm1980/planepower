% clear all
% close all
clc
         

init_lin_rotation


P.K = K;
P.xr = x;
P.u  = u;
P.Tf  = 5;

X0 = x;

intoptions.AbsTol = 1e-4;
intoptions.RelTol = 1e-4;
intoptions.MaxStep = 1/100;
intoptions.Events = 'off';


tic
[time,state]=ode45('Model_integ_ACADO_rotation_m',[0 P.Tf],X0,intoptions,P);
ET = toc;
RelTime = num2str(P.Tf/ET);
display(['Simulation time ratio: ',RelTime]);


figure
plot3(state(:,1),state(:,2),state(:,3))
axis equal
grid on
xlabel('x')
ylabel('y')
zlabel('z')
figure(2);
plot(state(:,1),state(:,2));
axis equal


