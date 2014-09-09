#!/usr/bin/env octave

if(nargin==0)
	R_control = 2^2;
	Q_alpha = 8^2;
	Q_dalpha = 5^2;
elseif(nargin==3)
	a = argv();
	R_control = a{1};
	Q_alpha = a{2};
	Q_dalpha = a{3};
else
	error('Wrong number of arguments!!! should be 0 or 3!')
end

stateVectorFieldNames={'delta_motor','delta_arm','alpha','beta', 'ddelta_motor','ddelta_arm','dalpha','dbeta', 'ddelta_motor_setpoint'};

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

write_binary = true;
write_cpf = false;

if write_binary
	disp('Writing out all of the precomputed data as binary to disk...');
	f = fopen('precomputedData.dat','w');

	timestamps = zeros(2,1);

	% NOTE: You must manually keep this in sync with PrecomputedData.h!
	fwrite(f,R_control);
	fwrite(f,Q_alpha);
	fwrite(f,Q_dalpha);
	fwrite(f,timestamps);
	
	fwrite(f,xss);
	fwrite(f,timestamps);

	fwrite(f,xss0);
	fwrite(f,timestamps);

	fwrite(f,xss1);
	fwrite(f,timestamps);
	
	fwrite(f,G);
	fwrite(f,timestamps);

	fclose(f);
end

if write_cpf
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

	function writeStateVector(f,names,values)
		for i=1:9
			writeval(f,names{i},values(i))
		end
	end

	f = fopen('lqr_gains.cpf','w');
	fprintf(f,'%s\n%s\n%s\n',h1,h2,h3);

	writeval(f,'R_control',R_control);
	writeval(f,'Q_alpha',Q_alpha);
	writeval(f,'Q_dalpha',Q_dalpha);
	writevector(f,'G_',G)
	fprintf(f,'%s\n',footer);
	fclose(f);

	disp('Writing out the steady states to .cpf files');
	stateVectorNames = {'xss','xss0','xss1'};
	stateVectors = {xss,xss0,xss1};
	for i=1:3
		stateVectorName = stateVectorNames{i};
		stateVector = stateVectors{i};
		f = fopen([stateVectorName '.cpf'],'w');
		fprintf(f,'%s\n%s\n%s\n',h1,h2,h3);
		writeStateVector(f,stateVectorFieldNames,stateVector);
		fprintf(f,'%s\n',footer);
	end
end
