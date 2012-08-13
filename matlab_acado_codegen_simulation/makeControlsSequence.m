clc
clear all

A = 10000;

T = 600;
Ts = 10;
ur = idinput(T*Ts,'prbs');
up = zeros(size(ur));

ur = A*ur;

U = [ur'; up'];

dlmwrite('sequence.dat',U,'delimiter',' ');