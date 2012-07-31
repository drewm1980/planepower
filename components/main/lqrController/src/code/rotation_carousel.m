function X_rotated = rotation_carousel(X)

%         R_delta = [cos(X(21)) -sin(X(21)) 0; ...
%                   sin(X(21)) cos(X(21)) 0; ...
%                    0           0           1];
%     
%         dR_delta = [-sin(X(21))  -cos(X(21))  0; ...
%                     cos(X(21)) -sin(X(21))  0; ...
%                      0              0          0];
% 
        R_delta = [cos(X(21)) sin(X(21)) 0; ...
                  -sin(X(21)) cos(X(21)) 0; ...
                   0           0           1];
 
        dR_delta = [-sin(X(21))  cos(X(21))  0; ...
                    -cos(X(21)) -sin(X(21))  0; ...
                     0              0          0];


                  dR_delta =  dR_delta*X(22);
               
        X_pos     = R_delta*X(1:3);
        X_vel     = (R_delta*X(4:6))+(dR_delta*X(1:3));
        R_r       = R_delta*[X(7:9)';X(10:12)';X(13:15)'];
      
        X_rotated = [X_pos; X_vel; R_r(1,:).'; R_r(2,:).'; R_r(3,:).'; X(16:end)];

end
