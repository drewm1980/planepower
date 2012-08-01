clear all;
close all;
clc
fid = fopen('data_a.dat','rt');
imu = fopen('imu.dat', 'w');
LEDpos = fopen('LEDpos.dat', 'w');
nLines = 0;
newline='a';
while (true),
    newline=fgets(fid);
    nLines = nLines+1;
    if nLines==1
        
        continue
        
    else
        index= strfind(newline,'nan');
        
        if index>115
            
            fprintf(imu, newline);
            
        else
            if newline ==-1
                break;
            else
                c2= regexp(newline, 'nan') ;
                if length(c2)==1
                    fprintf( LEDpos, newline);
                else
                    continue;
                end
            end
            
        end
    end
%keyboard
end
fclose(fid);
fclose(imu);
fclose(LEDpos);

% 
load('imu.dat');
load('LEDpos.dat');
X=imu;

% X = dlmread('dataClean.dat');
X = X(1:end-3,:);
t = X(:,1);

omegax = X(:,2);
omegay = X(:,3);
omegaz = X(:,4);


ax_meas = X(:,5);
ay_meas = X(:,6);
az_meas = X(:,7);

aileron_1 = X(:,8);
aileron_2 = X(:,9);
elevator  =X(:,10);

delta = X(:,11);
ddelta = X(:,12);

figure
subplot(311)
plot(t,omegax,'.')
title('t vs omegax')
xlabel('t')
title('omegax')

subplot(312)
plot(t,omegay,'.')
title('t vs omegay')
xlabel('t')
title('omegay')


subplot(313)
plot(t,omegaz,'.')
title('t vs omegaz')
xlabel('t')
title('omegaz')





figure
subplot(311)
plot(t,ax_meas)
subplot(312)
plot(t,ay_meas)
subplot(313)
plot(t,az_meas)
title('accelerations')

figure
subplot(311)
plot(t,aileron_1)
ylabel('aileron1')
subplot(312)
plot(t,aileron_2)
ylabel('aileron2')
subplot(313)
plot(t,elevator)
ylabel('elevator')

figure

plot(t,ddelta.*60/(2*pi))
ylabel('ddelta')
xlabel('time')

figure
hold on
plot(LEDpos(:,3),LEDpos(:,4),'or')

plot(LEDpos(:,5),LEDpos(:,6),'og')

plot(LEDpos(:,7),LEDpos(:,8),'ob')

ylabel('y pixel')
xlabel('x pixel')
title('Cam 1')
hold off;


figure
hold on
plot(LEDpos(:,9),LEDpos(:,10),'or')

plot(LEDpos(:,11),LEDpos(:,12),'og')

plot(LEDpos(:,13),LEDpos(:,14),'ob')

ylabel('y pixel')
xlabel('x pixel')
title('Cam 2')
hold off;


