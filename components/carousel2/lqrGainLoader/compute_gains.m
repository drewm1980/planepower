#!/usr/bin/env octave

if(nargin==0)
	R_control = 2^2;
	Q_alpha = 8^2;
	Q_dalpha = 5^2;
elseif(nargin==3)
	a = argv;
	R_control = argv{1}
	Q_alpha = argv{2}
	Q_dalpha = argv{3}
else
	error('Wrong number of arguments!!! should be 0 or 3!')
end

gainspath = '~/thilo/lin_data.mat';
disp('Loading linearised system data...');
load(gainspath,'A','B','C','xss','xss0','xss1');

n=9; % The number of states
assert(all(size(A)==[n,n]), 'A is an unexpected dimension!')
assert(all(size(B)==[n,1]), 'B is an unexpected dimension!')
assert(all(size(C)==[2,n]), ['C is an unexpected dimension!' num2str(size(C))])

Q = zeros(n,n);
Q(3,3) = Q_alpha;
Q(7,7) = Q_dalpha;

R = zeros(1,1);
R(1,1) = R_control;

disp('Recomputing gains...')
[G,X,L] = dlqr(A,B,Q,R);

disp('Writing out a .cpf file...')
h1='<?xml version="1.0" encoding="UTF-8"?>';
h2='<!DOCTYPE properties SYSTEM "cpf.dtd">';
h3='<properties>';
footer='</properties>';

function writeval(f,name,value)
	fprintf(f,'<simple name=\"%s\" type=\"double\"><value>%f</value></simple>\n',name,value)
end

function writevector(f,nameroot,values)
	for i=1:9
		writeval(f,[nameroot num2str(i-1)],values(i))
	end
end

f = fopen('gains_online.cpf','w');
fprintf(f,'%s\n%s\n%s\n',h1,h2,h3);

writeval(f,'R_control',R_control);
writeval(f,'Q_alpha',Q_alpha);
writeval(f,'Q_dalpha',Q_dalpha);

writevector(f,'K',G)
writevector(f,'Xss0_',G)
writevector(f,'Xss1_',G)

fprintf(f,'%s\n',footer);
fclose(f);

