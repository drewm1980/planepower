if 0,
% Script to produce an animation of the MHE horizon as it evolves.
loadData

fprintf('running computePrediction.m... \n')
computePrediction

fprintf('running Simulation.m... \n')
addpath Simulation
end

% Horizon data is in X_horizon
% N is the number of intervals in the MHE horizon
%  X_horizon has dimensions of NT x NX*((N+1)*3)

NT = size(X_horizon,1);

% State definition from loadData.m, for reference:
    %x_1 = X1(:,1); y_1 = X1(:,2); z_1 = X1(:,3); 
    %dx_1 = X1(:,4); dy_1 = X1(:,5); dz_1 = X1(:,6); 
    %e11_1 = X1(:,7); e12_1 = X1(:,8); e13_1 = X1(:,9); 
    %e21_1 = X1(:,10); e22_1 = X1(:,11); e23_1 = X1(:,12); 
    %e31_1 = X1(:,13); e32_1 = X1(:,14); e33_1 = X1(:,15); 
    %w1_1 = X1(:,16); w2_1 = X1(:,17); w3_1 = X1(:,18); 
    %delta_1 = X1(:,19); ddelta_1 = X1(:,20); 
    %ur_1 = X1(:,21); up_1 = X1(:,22);

dims = [NT,N+1,N_MHE_ITER,NX];
horizon = reshape(X_horizon,dims);

figure(1)
title('One state estimate over time')
plot(horizon(:,N+1,N_MHE_ITER,7))

figure(2)
title('MHE Horizon')

itercolors = ['r','g','b','k'];
% When we start the reporter, the MHE window is based on old data,
% so we wait a window's worth of iterations to start plotting.
%for t=N+1:NT 
for t=floor(NT/2) 
	clf(1)
	stateToPlot=11;
	hold off
	plot(horizon(t,:,1,stateToPlot))
	hold on
	for i=2:N_MHE_ITER
		plot(horizon(t,:,i,stateToPlot))
	end
end
	
