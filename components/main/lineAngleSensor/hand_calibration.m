%Script to calculate line angle sensor measurements.
% Made by Andrew physically moving tether manually
% while pranay gathered data.
% Note:  at a later date, Andrew trimmed the
% joystick so that the offsets are zero.
clear all;
close all;
clc;
%v=[vert axis distance from zero in mm, voltange in volts]
v =[     0    5.1781;
  -20.0000    3.0173;
  -40.0000    1.1827;
  -60.0000   -0.6930;
  -80.0000   -2.4093;
 -100.0000   -4.4002;
 -120.0000   -6.0184]

%h=[horiz axis distance from zero in mm, voltange in volts]
h=[      0   -0.1227;
   20.0000    1.2810;
   40.0000    2.5869;
   60.0000    3.8950]

horiz_run=360; %36 cm distance from tether attach point to tether

v_trig=atand(v(:,1)./horiz_run);
h_trig=atand(h(:,1)./horiz_run);

v_slope=(v(length(v),2)-v(1,2))/(v_trig(length(v_trig))-v_trig(1))
v_0=v(1,2)
h_slope=(h(length(h),2)-h(1,2))/(h_trig(length(h_trig))-h_trig(1))
h_0=h(1,2)

plot(v_trig,v(:,2));
xlabel('Degrees');ylabel('Voltage');
figure;
plot(h_trig,h(:,2));
xlabel('Degrees');ylabel('Voltage');
