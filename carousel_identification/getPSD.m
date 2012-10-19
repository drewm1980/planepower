close all
figure
subplot(311)
plot(data_aligned(:,1),data_aligned(:,5))
title('Accelerations')
ylabel('ax')
subplot(312)
plot(data_aligned(:,1),data_aligned(:,6))
ylabel('ay')
subplot(313)
plot(data_aligned(:,1),data_aligned(:,7))
ylabel('az')
xlabel('Time [s]')

figure
subplot(311)
plot(data_aligned(:,1),data_aligned(:,2))
title('Angular velocities')
ylabel('wx')
subplot(312)
plot(data_aligned(:,1),data_aligned(:,3))
ylabel('wy')
subplot(313)
plot(data_aligned(:,1),data_aligned(:,4))
ylabel('wz')
xlabel('Time [s]')


%pwelch(omega-mean(omega),[],[],[],500);
[Pxx_wx,F_wx] = pwelch(data_aligned(:,2)-mean(data_aligned(:,2)),[],[],[],500);
[Pxx_wy,F_wy] = pwelch(data_aligned(:,3)-mean(data_aligned(:,3)),[],[],[],500);
[Pxx_wz,F_wz] = pwelch(data_aligned(:,4)-mean(data_aligned(:,4)),[],[],[],500);
[Pxx_ax,F_ax] = pwelch(data_aligned(:,5)-mean(data_aligned(:,5)),[],[],[],500);
[Pxx_ay,F_ay] = pwelch(data_aligned(:,6)-mean(data_aligned(:,6)),[],[],[],500);
[Pxx_az,F_az] = pwelch(data_aligned(:,7)-mean(data_aligned(:,7)),[],[],[],500);

Pxx = [Pxx_wx Pxx_wy Pxx_wz Pxx_ax Pxx_ay Pxx_az];
F = [F_wx F_wy F_wz F_ax F_ay F_az];

figure
subplot(311)
hold on
plot(F(:,1),20*log10(Pxx(:,1)),'.')
plot(F(:,1),20*log10(Pxx(:,1)))
axis([0 20 min(20*log10(Pxx(:,1))) max(20*log10(Pxx(:,1)))])
ylabel('PSD of wx')
subplot(312)
hold on
plot(F(:,2),20*log10(Pxx(:,2)),'.')
plot(F(:,2),20*log10(Pxx(:,2)))
axis([0 20 min(20*log10(Pxx(:,2))) max(20*log10(Pxx(:,2)))])
ylabel('PSD of wy')
subplot(313)
hold on
plot(F(:,3),20*log10(Pxx(:,3)),'.')
plot(F(:,3),20*log10(Pxx(:,3)))
axis([0 20 min(20*log10(Pxx(:,3))) max(20*log10(Pxx(:,3)))])
ylabel('PSD of wz')
xlabel('Frequency [Hz]')

figure
subplot(311)
hold on
plot(F(:,4),20*log10(Pxx(:,4)),'.')
plot(F(:,4),20*log10(Pxx(:,4)))
axis([0 20 min(20*log10(Pxx(:,4))) max(20*log10(Pxx(:,4)))])
ylabel('PSD of ax')
subplot(312)
hold on
plot(F(:,5),20*log10(Pxx(:,5)),'.')
plot(F(:,5),20*log10(Pxx(:,5)))
axis([0 20 min(20*log10(Pxx(:,5))) max(20*log10(Pxx(:,5)))])
ylabel('PSD of ay')
subplot(313)
hold on
plot(F(:,6),20*log10(Pxx(:,6)),'.')
plot(F(:,6),20*log10(Pxx(:,6)))
axis([0 20 min(20*log10(Pxx(:,6))) max(20*log10(Pxx(:,6)))])
ylabel('PSD of az')
xlabel('Frequency [Hz]')