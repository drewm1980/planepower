%First load data and such
plotResults; %To load the data
getLQR; %To get the linearisation
%close all

scale_ur = 1.25e6;
scale_up = 2e5;

ur_scaled = ur/scale_ur;

e12_POSE_shifted = e11_POSE(3:end);
ur_scaled_shifted = ur_scaled(1:end-2);

[Txy,F]=tfestimate(ur_scaled_shifted,e12_POSE_shifted-mean(e12_POSE_shifted),300,[],[],10);

%%
for j=1:10
    %MPC.RD = 1e-2*2^(j-6);
    MPC.RD = 0.005;
    MPC.zT = -0.005*(j-1);
    %MPC.zT = -0.027;
    getLQR; %To get the linearisation
    A = MPC.A(1:18,1:18);
    B = MPC.A(1:18,19);
    C = zeros(1,18);
    C(8) = 1; % Corresponds to w1
    D = 0;
    sys = ss(A,B,C,D);
    [mag,phase,wout]=bode(sys);
    magg = [];
    for i=1:size(mag,3)
        magg = [magg;mag(1,1,i)];
    end
    mag_RD{j} = magg;
    wout_RD{j} = wout;
    RD{j} = MPC.RD;
    zT{j} = MPC.zT;
end

figure
hnew = semilogx(F,20*log10(abs(Txy)),'k');
legend('Measured tf')
[LEGH,OBJH,OUTH,OUTM] = legend;
hold on
for i=1:numel(mag_RD)
    if(mod(i,3))==0
        hnew = semilogx(wout_RD{i}/2/pi,20*log10(mag_RD{i}),'r');
    end
    if(mod(i,3))==1
        hnew = semilogx(wout_RD{i}/2/pi,20*log10(mag_RD{i}),'g');
    end
    if(mod(i,3))==2
        hnew = semilogx(wout_RD{i}/2/pi,20*log10(mag_RD{i}),'b');
    end
    legend([OUTH;hnew],OUTM{:},strcat('Model tf with zT: ', num2str(zT{i})))
    [LEGH,OBJH,OUTH,OUTM] = legend;
end
title('TF of e12 vs ur, measured by Andrews markers from pose')
x=[mean(ddelta_3)/2/pi];
ylim=get(gca,'ylim');
line([x;x],ylim.',...
         'linewidth',2,...
         'color',[0,1,0]);
     
  
%%
if(0)
y_POSE_shifted = y_POSE(3:end);
[Txy,F]=tfestimate(ur_scaled_shifted,y_POSE_shifted-mean(y_POSE_shifted),300,[],[],10);

A = MPC.A(1:18,1:18);
B = MPC.A(1:18,19);
C = zeros(1,18);
C(2) = 1; % Corresponds to e12
D = 0;
sys = ss(A,B,C,D);
[mag,phase,wout]=bode(sys);
magg = [];
for i=1:size(mag,3)
    magg = [magg;mag(1,1,i)];
end

figure
semilogx(F,20*log10(abs(Txy)),'b')
hold on
semilogx(wout/2/pi,20*log10(magg),'r')
title('TF of y vs ur, measured by Andrews markers from pose')
legend('Measured tf','model tf')
x=[mean(ddelta_3)/2/pi];
ylim=get(gca,'ylim');
line([x;x],ylim.',...
         'linewidth',2,...
         'color',[0,1,0]);     
%%
z_POSE_shifted = z_POSE(3:end);
[Txy,F]=tfestimate(ur_scaled_shifted,z_POSE_shifted-mean(z_POSE_shifted),300,[],[],10);

A = MPC.A(1:18,1:18);
B = MPC.A(1:18,19);
C = zeros(1,18);
C(2) = 1; % Corresponds to e12
D = 0;
sys = ss(A,B,C,D);
[mag,phase,wout]=bode(sys);
magg = [];
for i=1:size(mag,3)
    magg = [magg;mag(1,1,i)];
end

figure
semilogx(F,20*log10(abs(Txy)),'b')
hold on
semilogx(wout/2/pi,20*log10(magg),'r')
title('TF of z vs ur, measured by Andrews markers from pose')
legend('Measured tf','model tf')
x=[mean(ddelta_3)/2/pi];
ylim=get(gca,'ylim');
line([x;x],ylim.',...
         'linewidth',2,...
         'color',[0,1,0]);
%%
[Txy,F]=tfestimate(ur_scaled,ddelta_3-mean(ddelta_3),[],[],[],10);

figure
semilogx(F,20*log10(abs(Txy)),'b')
title('TF of z vs ur, measured by Andrews markers from pose')
legend('Measured tf')
x=[mean(ddelta_3)/2/pi];
ylim=get(gca,'ylim');
line([x;x],ylim.',...
         'linewidth',2,...
         'color',[0,1,0]);
     
%%
 figure
 subplot(311)
 hold on
 xlabel('time')
 ylabel('x')
 plot(t,x_POSE)
 plot(t,x_POSE,'.')
 subplot(312)
 hold on
 xlabel('time')
 ylabel('y')
 plot(t,y_POSE)
 plot(t,y_POSE,'.')
 subplot(313)
 hold on
 xlabel('time')
 ylabel('z')
 plot(t,z_POSE)
 plot(t,z_POSE,'.')
end
