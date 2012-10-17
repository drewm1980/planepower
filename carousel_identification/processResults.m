aIMU_res = dlmread('aIMU_res.dat');
wIMU_res = dlmread('wIMU_res.dat');
aIMU_pred = dlmread('aIMU_pred.dat');
wIMU_pred = dlmread('wIMU_pred.dat');
aIMU_meas = dlmread('aIMU.dat');
wIMU_meas = dlmread('wIMU.dat');

t = 0:0.002:0.002*(size(aIMU_res,1)-1);

figure
subplot(311)
plot(t,aIMU_res(:,1))
ylabel('ax')
subplot(312)
plot(t,aIMU_res(:,2))
ylabel('ay')
subplot(313)
plot(t,aIMU_res(:,3))
ylabel('az')
xlabel('time')

figure
subplot(311)
plot(t,wIMU_res(:,1))
ylabel('wx')
subplot(312)
plot(t,wIMU_res(:,2))
ylabel('wy')
subplot(313)
plot(t,wIMU_res(:,3))
ylabel('wz')
xlabel('time')

figure
subplot(311)
pwelch(aIMU_res(:,1),[],[],[],500)
subplot(312)
pwelch(aIMU_res(:,2),[],[],[],500)
subplot(313)
pwelch(aIMU_res(:,3),[],[],[],500)

figure
subplot(311)
pwelch(wIMU_res(:,1),[],[],[],500)
subplot(312)
pwelch(wIMU_res(:,2),[],[],[],500)
subplot(313)
pwelch(wIMU_res(:,3),[],[],[],500)



figure
subplot(311)
hold on
plot(t,aIMU_pred(:,1),'r')
plot(t,aIMU_meas(:,1),'b')
legend('Predicted','Measured')
ylabel('ax')
subplot(312)
hold on
plot(t,aIMU_pred(:,2),'r')
plot(t,aIMU_meas(:,2),'b')
ylabel('ay')
subplot(313)
hold on
plot(t,aIMU_pred(:,3),'r')
plot(t,aIMU_meas(:,3),'b')
ylabel('az')
xlabel('time')

figure
subplot(311)
hold on
plot(t,wIMU_pred(:,1),'r')
plot(t,wIMU_meas(:,1),'b')
legend('Predicted','Measured')
ylabel('wx')
subplot(312)
hold on
plot(t,wIMU_pred(:,2),'r')
plot(t,wIMU_meas(:,2),'b')
ylabel('wy')
subplot(313)
hold on
plot(t,wIMU_pred(:,3),'r')
plot(t,wIMU_meas(:,3),'b')
ylabel('wz')
xlabel('time')


%%
[Pxx_ax_res,F_ax_res] = pwelch(aIMU_res(:,1),[],[],[],500);
[Pxx_ax_meas,F_ax_meas] = pwelch(aIMU_meas(:,1)-mean(aIMU_meas(:,1)),[],[],[],500);
[Pxx_ay_res,F_ay_res] = pwelch(aIMU_res(:,2),[],[],[],500);
[Pxx_ay_meas,F_ay_meas] = pwelch(aIMU_meas(:,2)-mean(aIMU_meas(:,2)),[],[],[],500);
[Pxx_az_res,F_az_res] = pwelch(aIMU_res(:,3),[],[],[],500);
[Pxx_az_meas,F_az_meas] = pwelch(aIMU_meas(:,3)-mean(aIMU_meas(:,3)),[],[],[],500);
[Pxx_wx_res,F_wx_res] = pwelch(wIMU_res(:,1),[],[],[],500);
[Pxx_wx_meas,F_wx_meas] = pwelch(wIMU_meas(:,1)-mean(wIMU_meas(:,1)),[],[],[],500);
[Pxx_wy_res,F_wy_res] = pwelch(wIMU_res(:,2),[],[],[],500);
[Pxx_wy_meas,F_wy_meas] = pwelch(wIMU_meas(:,2)-mean(wIMU_meas(:,2)),[],[],[],500);
[Pxx_wz_res,F_wz_res] = pwelch(wIMU_res(:,3),[],[],[],500);
[Pxx_wz_meas,F_wz_meas] = pwelch(wIMU_meas(:,3)-mean(wIMU_meas(:,3)),[],[],[],500);
Pxx_res = [Pxx_ax_res Pxx_ay_res Pxx_az_res Pxx_wx_res Pxx_wy_res Pxx_wz_res];
Pxx_meas = [Pxx_ax_meas Pxx_ay_meas Pxx_az_meas Pxx_wx_meas Pxx_wy_meas Pxx_wz_meas];
F_res = [F_ax_res F_ay_res F_az_res F_wx_res F_wy_res F_wz_res];
F_meas = [F_ax_meas F_ay_meas F_az_meas F_wx_meas F_wy_meas F_wz_meas];

figure
hold on
plot(F_res(:,1),20*log10(Pxx_res(:,1)),'r')
plot(F_meas(:,1),20*log10(Pxx_meas(:,1)),'b')
axis([0 20 min([20*log10(Pxx_res(:,1));20*log10(Pxx_meas(:,1))]) max([20*log10(Pxx_res(:,1));20*log10(Pxx_meas(:,1))])])
xlabel('Frequency [Hz]')
ylabel('Amplitue [dB]')
title('PSD of ax')
legend('spectra of the residual','Measured signal')