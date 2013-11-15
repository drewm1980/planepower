%Script to calculate line angle sensor measurements.
% Made by Andrew physically moving tether manually
% while pranay gathered data.
% Note:  at a later date, Andrew trimmed the
% joystick so that the offsets are zero.
clear all;
close all;
clc;

%v=[vert axis distance from zero in mm, voltage in volts]
v =[     0    5.1781;
  -20.0000    3.0173;
  -40.0000    1.1827;
  -60.0000   -0.6930;
  -80.0000   -2.4093;
 -100.0000   -4.4002;
 -120.0000   -6.0184]

%h=[horiz axis distance from zero in mm, voltage in volts]
h=[      0   -0.1227;
   20.0000    1.2810;
   40.0000    2.5869;
   60.0000    3.8950]

horiz_run=360; %36 cm distance from tether attach point to tether

v_trig=atan(v(:,1)./horiz_run);
h_trig=atan(h(:,1)./horiz_run);

v_0=v(1,2) % volts
h_0=h(1,2)
dv_v = v(:,2)-v_0;
dv_h = h(:,2)-h_0;
v_slopes = v_trig./dv_v; % radians / V
h_slopes = h_trig./dv_h; % radians / V

v_slope = mean(v_slopes(2:end)) % radians / V
h_slope = mean(h_slopes(2:end)) % radians / V


%plot(v_trig,v(:,2));
%xlabel('Degrees');ylabel('Voltage');
%figure;
%plot(h_trig,h(:,2));
%xlabel('Degrees');ylabel('Voltage');

disp('--------------------- REDO with Joris -----------------------')

% Horizontal:
% (mm of displacement) volts
h = [ 0, 0.53 
-60, -5.365 % behind rotation
60,  5.5]; % ahead of rotation

% Vertical:
% mm volts
v = [0,  1.22
42.7, 6.5 % up
-60, -7.1]; % down

% Lever arm for both cases:
horiz_run = 243; %%mm

% Offsets
v_0=v(1,2) % volts
h_0=h(1,2)

% Meaning of the offsets:
% theta_radians = (v-v_0)* v_slope

v_trig=atan(v(:,1)./horiz_run); % radians
h_trig=atan(h(:,1)./horiz_run); % radians
dv_v = v(:,2)-v_0;
dv_h = h(:,2)-h_0;
v_slopes = v_trig./dv_v; % radians / V
h_slopes = h_trig./dv_h; % radians / V

v_slope = mean(v_slopes(2:end)) % radians / V
h_slope = mean(h_slopes(2:end)) % radians / V

%v_0 =  1.2200
%h_0 =  0.53000
%v_slope =  0.031020
%h_slope =  0.044885


