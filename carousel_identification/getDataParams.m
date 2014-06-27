loadData;

% Compute the gravity constant
g = (data_aligned_zero(:,5).^2+data_aligned_zero(:,6).^2+data_aligned_zero(:,7).^2).^(1/2);
g = mean(g);

% Compute the angular velocity, by removing outliers
ddelta_t = data_aligned(:,9);
ddelta_var = std(ddelta_t);
ddelta_treshold = 3*ddelta_var>abs(ddelta_t-mean(ddelta_t));
ddelta = ddelta_t'*ddelta_treshold/sum(ddelta_treshold);
omega = zeros(size(data_aligned,1),1);

for i=1:numel(omega)
    omega(i) = norm(data_aligned(i,2:4));
end
t = 0:1/500:1/500*(numel(omega)-1);
fs = 500;
[b,a] = butter(4,1/(fs/2))
close all
y = filter(b,a,omega-mean(omega));
figure(1)
hold on
plot(t,omega,'r')
plot(t,y+mean(omega))

dlmwrite('ddelta_t_004.dat',ddelta_t,'delimiter',' ','precision',10);
dlmwrite('g_004.dat',g);
dlmwrite('ddelta_004.dat',ddelta);
dlmwrite('delta_004.dat',data_aligned(:,8),'delimiter',' ','precision',10);
dlmwrite('aIMU_004.dat',data_aligned(:,5:7),'delimiter',' ','precision',10);
dlmwrite('wIMU_004.dat',data_aligned(:,2:4),'delimiter',' ','precision',10);