close all
clear all
clc

load DataSim
Ncvp = 40;

% x = state(1);
% y = state(2);
% z = state(3);
% 
% dx = state(4);
% dy = state(5);
% dz = state(6);
% 
% q0 = state(7);
% q1 = state(8);
% q2 = state(9);
% q3 = state(10);
% 
% w1 = state(11);
% w2 = state(12);
% w3 = state(13);
% 
% r = state(14);
% dr = state(15);
% 
% E = state(16);
% dn = state(17);
% 
% u = linint(P.tu,t);
% ddr = u(1);
% w1_ref = u(2);
% w2_ref = u(3);
% w3_ref = u(4);

StateVar = {'x','y','z','dx','dy','dz','q0','q1','q2','q3','w1','w2','w3','r','dr','delta','ddelta'};
CtrVar = {'dddelta','ddr','u1','u2','F1per','F2per','F3per','T1per','T2per','T3per'};
ParamVar = {'RD'};

MeasVar = {'ax','ay','az','w_1','w_2','w_3','r','dr','delta','ddelta','dddelta','ddr','u1','u2'};

twrite = linspace(0,tx(end,1),Ncvp+1)';
twritescale = twrite;

statewrite = linint(tx(:,1:length(StateVar)+1),twrite)';
uwrite = zeros(length(twrite),10);
paramwrite = linint(tp,twrite)';
paramwrite = [paramwrite];

%OUTPUT ax ay az q0...q3 w1 w2 w3 r dr delta ddelta

%MEASUREMENT NOISE
STDIMU = 0.2; %Signal-to-noise ratio
tyNoise = ty;

for k = 1:6
    Noise = std(ty(:,k+1));
    tyNoise(:,k+1) = tyNoise(:,k+1) + random('norm',0,Noise*STDIMU,size(tyNoise,1),1);
end

ywriteLin = linint(tyNoise,twrite)';

if (STDIMU > 0)
%SP = csaps(tyNoise(:,1),tyNoise(:,2:end)',0.995)';
%Tol = 3.5; %0.5 STD
Tol = 1; %0.3 STD
%Tol = 0.01; %0.1 STD

SP = spaps(tyNoise(:,1),tyNoise(:,2:end)',Tol,[],3);
ywrite = fnval(SP,twrite)';
else
    ywrite = ywriteLin;
end
    
  
figure(10)
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])

for k = 1:9
    subplot(3,3,k)
    plot(tyNoise(:,1),tyNoise(:,k+1),'color','b');hold on
    plot(ty(:,1),ty(:,k+1),'color','k','linewidth',2);hold on
    plot(twrite,ywrite(:,k),'color','r','linestyle','none','marker','.','markersize',15);hold on
    plot(twrite,ywriteLin(:,k),'color','g','linestyle','none','marker','.','markersize',15);hold on
end
legend('Noisy data','Noise-free data','Splined','Linear interpolation')

save RefTrajMat twrite ywrite uwrite statewrite

form = [];
for k = 1:length(StateVar)+1;
form = [form,' %6.16e'];
end
form = [form,' \r\n'];
fid = fopen('../powerkite_states0.txt', 'w');
fprintf(fid,form,[twritescale statewrite]');
fclose(fid);

form = [];
for k = 1:length(CtrVar)+1;
form = [form,' %6.16e'];
end
form = [form,' \r\n'];
fid = fopen('../powerkite_controls0.txt', 'w');
fprintf(fid,form,[twritescale uwrite ]');
fclose(fid);

form = [];
for k = 1:length(ParamVar)+1;
form = [form,' %6.16e'];
end
form = [form,' \r\n'];
fid = fopen('../powerkite_params0.txt', 'w');
fprintf(fid,form,[twritescale paramwrite]');
fclose(fid);



form = [];
for k = 1:size(ywrite,2)+size(uwrite,2)+1;
form = [form,' %6.16e'];
end
form = [form,' \r\n'];
fid = fopen('../RefTraj.txt', 'w');
fprintf(fid,form,[twritescale ywrite uwrite]');
fclose(fid);

figure(3);
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])

NP = ceil(sqrt(length(ParamVar)));
for k = 1:length(ParamVar)-2
    subplot(NP,NP,k)
    plot(tp(:,1),tp(:,k+1),'linewidth',2);hold on
    plot(twrite(:),paramwrite(:,k),'linewidth',2,'color','k','linestyle','--');hold on
    grid
    ylabel(ParamVar{k})
end
title('Parameters')

figure(1);
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])

NP = ceil(sqrt(length(StateVar)));
for k = 1:length(StateVar)
    subplot(NP,NP,k)
    plot(tx(:,1),tx(:,k+1),'linewidth',2);hold on
    plot(twrite(:),statewrite(:,k),'linewidth',2,'color','k','linestyle','--');hold on
    grid
    ylabel(StateVar{k})
end

figure(2);
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])

NP = ceil(sqrt(length(CtrVar)-6));
for k = 1:length(CtrVar)-6
    subplot(NP,NP,k)
   % plot(turef(:,1),turef(:,k+1),'linewidth',2);hold on
    plot(twrite(:),uwrite(:,k),'linewidth',2,'color','k','linestyle','--');hold on
    grid
    ylabel(CtrVar{k})
end


figure(4);
 whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])

NP = ceil(sqrt(length(MeasVar)));
for k = 1:size(ywrite,2)
    subplot(NP,NP,k)
    plot(ty(:,1),ty(:,k+1),'linewidth',2);hold on
    plot(twrite(:),ywrite(:,k),'linewidth',2,'color','k','linestyle','--');hold on
    grid
    ylabel(MeasVar{k})
end
for k = size(ywrite,2)+1:size(ywrite,2)+size(uwrite,2)-6
    subplot(NP,NP,k)
    %plot(turef(:,1),turef(:,k-size(ywrite,2)+1),'linewidth',2);hold on
    plot(twrite(:),uwrite(:,k-size(ywrite,2)),'linewidth',2,'color','k','linestyle','--');hold on
    grid
    ylabel(MeasVar{k})
end
