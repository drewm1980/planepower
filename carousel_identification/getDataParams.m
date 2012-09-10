%loadData;

% Compute the gravity constant
g = (data_aligned_zero(:,5).^2+data_aligned_zero(:,6).^2+data_aligned_zero(:,7).^2).^(1/2);
g = mean(g);

% Compute the angular velocity, by removing outliers
ddelta_t = data_aligned(:,9);
ddelta_var = std(ddelta_t);
ddelta_treshold = 3*ddelta_var>abs(ddelta_t-mean(ddelta_t));
ddelta = ddelta_t'*ddelta_treshold/sum(ddelta_treshold);

dlmwrite('g.dat',g);
dlmwrite('ddelta.dat',ddelta);
dlmwrite('delta.dat',data_aligned(:,8),'delimiter',' ','precision',10);
dlmwrite('aIMU.dat',data_aligned(:,5:7),'delimiter',' ','precision',10);
dlmwrite('wIMU.dat',data_aligned(:,2:4),'delimiter',' ','precision',10);