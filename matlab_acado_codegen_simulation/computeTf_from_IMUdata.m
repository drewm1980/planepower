%First load data and such
clear all
getIMU_controlSequence; %To load the data
%close all



[Txy,F]=tfestimate(ur_scaled,w1-mean(w1),[],[],[],500);
%%
for j=1:9
    MPC.RD = 1e-2*2^(j-6);
    %MPC.RD = 0.005;
    %MPC.zT = -0.005*(j-1);
    MPC.zT = -0.027;
    %MPC.I1 = 0.0163*j*5;
    MPC.I1 = 0.0163;
    getLQR; %To get the linearisation
    A = MPC.A(1:18,1:18);
    B = MPC.A(1:18,19);
    C = zeros(1,18);
    C(16) = 1; % Corresponds to w1
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
    I1{j} = MPC.I1;
end
%%
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
    legend([OUTH;hnew],OUTM{:},strcat('Model tf with RD: ', num2str(RD{i})))
    [LEGH,OBJH,OUTH,OUTM] = legend;
end
title('TF of omega_1 vs ur, measured by IMU')
x=37/60;
ylim=get(gca,'ylim');
line([x;x],ylim.',...
         'linewidth',2,...
         'color',[0,1,0]);
