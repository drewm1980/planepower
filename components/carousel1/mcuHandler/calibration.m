% This code contains calibration measurements and math for control surfaces.


% radians = (volt - offset) * scale

% There is no measurable offset; zero voltage == zero angular position

% lever arm lengths right_aileron left_aileron elevator
arm = [26.8 27.3 46.0]


% Deflection in mm wrt to zero position
pos1 = [ 8.1 7.7 7.9 ]
pos2 = [ 8.6 8.0 15.4]

% Voltages
volt1 = [-0.6 -0.6 0.2]
volt2 = [0.6 0.6 -0.4]


rad1 = atan2(pos1,arm)
rad2 = atan2(pos2,arm)

scale1 = rad1./volt1
scale2 = rad2./volt2

disp('rad/volt')
scale = mean([abs(scale1);abs(scale2)],1)

% Right aileron height: in mm, measured 
% at trailing edge of wing, at point of aileron closest to fuselage
% between the bottom surface of the aileron to the bottom surface of the fuselage.
% starting at 0.4 we switch to top-top measurements.


% unitless_command	right_aileron_height left_aileron_height elevator_height
%-.6	 8.1 7.7
%0   
%.6  8.6 8.0


% Reference: infinite ruler between wing and tail from the bottom, distance delta w.r.t. unitless_command=0
%0.2 7.9
%-0.4 15.4

