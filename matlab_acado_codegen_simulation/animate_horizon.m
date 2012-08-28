% Script to produce an animation of the MHE horizon as it evolves.
loadData
computePrediction

addpath Simulation

% Horizon data is in X_horizon
% N is the number of intervals in the MHE horizon
%  X_horizon has dimensions of NT x NX*((N+1)*3)

NT = size(X_horizon,1);

figure(1)
title('MHE Horizon')
dims = [NT,N+1,N_MHE_ITER,NX];
horizon = reshape(X_horizon,dims);

horizon_imuaccel = horizon(:,:,:,)

itercolors = ['r','g','b','k']
for t=N+1:NT % When we start the reporter, the MHE window is based on old data,
				% so we wait a window's worth of iterations to start plotting.
	for i=1:N_MHE_ITER
		iterslice = timeslice
		plot()
	end
end
	
