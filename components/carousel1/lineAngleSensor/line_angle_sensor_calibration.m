%Script to calculate line angle sensor measurements.
% Made by Andrew physically moving tether manually
% while pranay gathered data.
% Note:  at a later date, Andrew trimmed the
% joystick so that the offsets are zero.
clear all;
close all;
clc;

disp('--------------------- REDO with Andrew and Kurt -----------------------')

% Horizontal:
% (mm of displacement) volts
h = [ 0, 0.69 
-60, -4.96 % behind rotation
60,  3.473]; % ahead of rotation
horiz_run_for_horizontal = 372; %%mm

% Vertical:
% mm volts
v = [0, -.7 
42, 4.24 % up
-60, -7.34]; % down
% Lever arm for vertical case:
horiz_run = 295; %%mm

% Offsets
v_0=v(1,2) % volts
h_0=h(1,2)

% Meaning of the offsets:
% theta_radians = (v-v_0)* v_slope

v_trig=atan(v(:,1)./horiz_run); % radians
h_trig=atan(h(:,1)./horiz_run_for_horizontal); % radians
dv_v = v(:,2)-v_0;
dv_h = h(:,2)-h_0;
v_slopes = v_trig./dv_v; % radians / V
h_slopes = h_trig./dv_h; % radians / V

v_slope = mean(v_slopes(2:end)) % radians / V
h_slope = mean(h_slopes(2:end)) % radians / V

% From Andrew and Joris
%v_0 =  1.2200
%h_0 =  0.53000
%v_slope =  0.031020
%h_slope =  0.044885

%--------------------- REDO with Andrew and Kurt -----------------------
%v_0 = -0.70000
%h_0 =  0.69000
%v_slope =  0.029423  % positive angle is UP in real world
%h_slope =  0.042882  % positive angle is AHEAD of rotation in real world


