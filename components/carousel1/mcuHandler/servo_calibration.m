% This code contains calibration measurements and math for control surfaces.

% radians = (control - offset) * scale

% There is no measurable offset; zero controlage == zero angular position

% Right aileron height: in mm, measured 
% at trailing edge of wing, at point of aileron closest to fuselage
% between the bottom surface of the aileron to the bottom surface of the wing.

% lever arm lengths: 
%    right_aileron left_aileron elevator
arm = [27 27 10.5] 

% unitless control values
control1 = [.2 .2 .2]
control2 = [.4 .4 .4]

% Deflection in mm wrt to zero position
pos1 = [ 2.5 2.5 1.7]
pos2 = [ 5.2 5.2 3.3]

rad1 = atan2(pos1,arm)
rad2 = atan2(pos2,arm)

scale1 = rad1./control1
scale2 = rad2./control2

disp('rad/control')
scale = mean([abs(scale1);abs(scale2)],1)

