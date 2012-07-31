function sys = Model_integ_ACADO(t,state,flag,P)Parametersx = state(1);y = state(2);z = state(3);dx = state(4);dy = state(5);dz = state(6);e11 = state(7);    e12 = state(8);      e13 = state(9);          e21 = state(10);     e22 = state(11);      e23 = state(12);         e31 = state(13);     e32 = state(14);      e33 = state(15);  w1 = state(16);w2 = state(17);w3 = state(18);delta = P.delta;%state(19);ddelta = P.ddelta;%state(20);u1 = state(19);u2 = state(20);ur = u1;up = u2; u = linint(P.tu,t);dddelta = P.dddelta;%u(1);% ddr = u(2);du1 = u(1);du2 = u(2);r = P.r;dr = 0;	% DEFINITION OF PI :	% ------------------------		 PI = 3.1415926535897932;			  	% CONSTANTS :% ------------------------		%  PARAMETERS OF THE KITE :		%  -----------------------------		 mk =  0.626;      %  mass of the kite               %  [ kg    ]		%  A =  0.2;      %  effective area                 %  [ m^2   ]		%   PHYSICAL CONSTANTS :		%  -----------------------------		   g =    9.81;      %  gravitational constant         %  [ m /s^2]		 rho =    1.23;      %  density of the air             %  [ kg/m^3]		%  PARAMETERS OF THE CABLE :		%  -----------------------------		 rhoc = 1450.00;      %  density of the cable           %  [ kg/m^3]% 		 cc =   1.00;      %  frictional constant            %  [       ]		 dc = 1e-3;      %  diameter                       %  [ m     ]		 AQ      =  PI*dc*dc/4.0;		%CAROUSEL ARM LENGTH		 rA = 1.085; %(dixit Kurt)         zT = -0.02;% 		 XT = [0;0;-0.01];% 		 ZT = 0;		%             YT = 0.005;		%INERTIA MATRIX (Kurt's direct measurements)		 I1 = 0.0163;		 I31 = 0.0006;		 I2 = 0.0078;		 I3 = 0.0229;		%IMU POSITION & ANGLE		 XIMU1 = 0.0246;         XIMU2 = -0.0116;         XIMU3 = -0.0315;% 		 alphaIMU = 0*pi/180;%4% 		 betaIMU = 0*pi/180;% 		 deltaIMU = 0*pi/180;% 		 alpha0 = -0*PI/180; 		%TAIL LENGTH		 LT = 0.4;		%ROLL DAMPING		 RD = 1e-2; 		 PD = 0*1e-3;		 YD = 0*1e-3;		%WIND-TUNNEL PARAMETERS		%Lift (report p. 67)		 CLA = 5.064;		 CLe = -1.924;%e-5;%0.318;%		 CL0 = 0.239;		%Drag (report p. 70)		 CDA = -0.195;		 CDA2 = 4.268;		 CDB2 = 5;%0;%% 		 CDe = 0.044;% 		 CDr = 0.111;		 CD0 = 0.026;		%Roll (report p. 72)		 CRB = -0.062;		 CRAB = -0.271; 		 CRr = -5.637e-1;%e-6;%-0.244;%		%Pitch (report p. 74)		 CPA = 0.293;		 CPe = -4.9766e-1;%e-6;%-0.821;%		 CP0 = 0.03;		%Yaw (report p. 76)		 CYB = 0.05;		 CYAB = 0.229;		 SPAN = 0.96;		 CHORD = 0.1;%                        MODEL EQUATIONS :% ===============================================================	% CROSS AREA OF THE CABLE :	% ---------------------------------------------------------------	% 	AQ      =  PI*dc*dc/4.0                                       ;		% THE EFECTIVE MASS' :	% ---------------------------------------------------------------		mc      =  0*rhoc*AQ*r  ;   % mass of the cable	m       =  mk + mc/3.0;   % effective inertial mass	mgrav   =  mk + mc/2.0;   % effective inertial mass		% -----------------------------   % ----------------------------	%   dm      =  (rhoc*AQ/ 3.0)*dr;   % time derivative of the mass			% WIND SHEAR MODEL :	% ---------------------------------------------------------------		wind       =  0.;			% EFFECTIVE WIND IN THE KITE`S SYSTEM :	% ---------------------------------------------------------------		we(1)   = -wind + dx - ddelta*y;	we(2)   =		  dy + ddelta*rA + ddelta*x;	we(3)   =		  dz;		VKite2 = (we(1)*we(1) + we(2)*we(2) + we(3)*we(3)); 	VKite = sqrt(VKite2); 		% CALCULATION OF THE FORCES :	% ---------------------------------------------------------------	% 	% er%     er(1) = x/r;% 	er(2) = y/r;% 	er(3) = z/r;% 	% 	%Velocity accross X (cable drag)% 	wp = er(1)*we(1) + er(2)*we(2) + er(3)*we(3);% 	wep(1) = we(1) - wp*er(1);% 	wep(2) = we(2) - wp*er(2);% 	wep(3) = we(3) - wp*er(3);		%Aero coeff.			% LIFT DIRECTION VECTOR	% -------------------------	%Relative wind speed in Airfoil's referential 'E'	wE(1) = e11*we(1)  + e12*we(2)  + e13*we(3);	wE(2) = e21*we(1)  + e22*we(2)  + e23*we(3);	wE(3) = e31*we(1)  + e32*we(2)  + e33*we(3);    	wE(1) = e11*we(1)  + e21*we(2)  + e31*we(3);	wE(2) = e12*we(1)  + e22*we(2)  + e32*we(3);	wE(3) = e13*we(1)  + e23*we(2)  + e33*we(3);	%Airfoil's transversal axis in fixed referential 'e'	eTe(1) = e21;	eTe(2) = e22;	eTe(3) = e23;    	eTe(1) = e12;	eTe(2) = e22;	eTe(3) = e32;	% Lift axis ** Normed to we !! **	eLe(1) = - eTe(2)*we(3) + eTe(3)*we(2);	eLe(2) = - eTe(3)*we(1) + eTe(1)*we(3);	eLe(3) = - eTe(1)*we(2) + eTe(2)*we(1);	% AERODYNAMIC COEEFICIENTS	% ----------------------------------	%VT = cross([w1;w2;w3],[-LT;0;0]) + wE;	VT(1) =          wE(1);	VT(2) = -LT*w3 + wE(2);	VT(3) =  LT*w2 + wE(3);	alpha = -wE(3)/wE(1);	%Note: beta & alphaTail are compensated for the tail motion induced by omega% 	beta = VT(2)/sqrt(VT(1)*VT(1) + VT(3)*VT(3));	betaTail = VT(2)/VT(1);%     betaTail = wE(2)/sqrt(wE(1)*wE(1) + wE(3)*wE(3));    beta = wE(2)/wE(1);	alphaTail = -VT(3)/VT(1);% 	CL = CLA*alpha + CLe*up     + CLr*ur + CL0;% 	CD = CDA*alpha + CDA2*alpha*alpha + CDB2*beta*beta + CDe*up + CDr*ur + CD0;% 	CR = -RD*w1 + CRB*betaTail + CRAB*alphaTail*betaTail + CRr*ur;% 	CP = CPA*alphaTail + CPe*up + CPr*ur + CP0;% 	CY = CYB*betaTail + CYAB*alphaTail*betaTail;	CL = CLA*alpha + CLe*up + CL0;	CD = CDA*alpha + CDA2*alpha*alpha + CDB2*beta*beta + CD0;	CR = -RD*w1 + CRB*betaTail + CRr*ur + CRAB*alphaTail*betaTail;	CP = -PD*w2 + CPA*alphaTail + CPe*up + CP0;	CY = -YD*w3 + CYB*betaTail + CYAB*alphaTail*betaTail;			% 	Cf = rho*dc*r*VKite/8.0;	% THE FRICTION OF THE CABLE :	% ---------------------------------------------------------------% 	Ff(1) = -rho*dc*r*VKite*cc*wep(1)/8.0;% 	Ff(2) = -rho*dc*r*VKite*cc*wep(2)/8.0;% 	Ff(3) = -rho*dc*r*VKite*cc*wep(3)/8.0;	% LIFT :	% ---------------------------------------------------------------    CL = 0.2*CL;	FL(1) =  rho*CL*eLe(1)*VKite/2.0;	FL(2) =  rho*CL*eLe(2)*VKite/2.0;	FL(3) =  rho*CL*eLe(3)*VKite/2.0;	% DRAG :	% ---------------------------------------------------------------	FD(1) = -rho*VKite*CD*we(1)/2.0;	FD(2) = -rho*VKite*CD*we(2)/2.0; 	FD(3) = -rho*VKite*CD*we(3)/2.0; 	% FORCES (AERO)	% ---------------------------------------------------------------	F(1) = FL(1) + FD(1);	F(2) = FL(2) + FD(2);	F(3) = FL(3) + FD(3);	% ATTITUDE DYNAMICS	% -----------------------------------------------------------% 	de11 = e21*w3 - e31*w2 + ddelta*e12; % 	de21 = e31*w1 - e11*w3 + ddelta*e22; % 	de31 = e11*w2 - e21*w1 + ddelta*e32; % 	de12 = e22*w3 - e32*w2 - ddelta*e11; % 	de22 = e32*w1 - e12*w3 - ddelta*e21; % 	de32 = e12*w2 - e22*w1 - ddelta*e31; % 	de13 = e23*w3 - e33*w2; % 	de23 = e33*w1 - e13*w3; % 	de33 = e13*w2 - e23*w1;         R = [e11 e12 e13;         e21 e22 e23;         e31 e32 e33];    RotPole = 1/2;    RP = RotPole*R*(inv(R.'*R) - eye(3));        de11 = e12*w3 - e13*w2 + ddelta*e21  + RP(1,1);    de12 = e13*w1 - e11*w3 + ddelta*e22  + RP(1,2);    de13 = e11*w2 - e12*w1 + ddelta*e23  + RP(1,3);    de21 = e22*w3 - e23*w2 - ddelta*e11  + RP(2,1);    de22 = e23*w1 - e21*w3 - ddelta*e12  + RP(2,2);     de23 = e21*w2 - e22*w1 - ddelta*e13  + RP(2,3);    de31 = e32*w3 - e33*w2               + RP(3,1);    de32 = e33*w1 - e31*w3               + RP(3,2);    de33 = e31*w2 - e32*w1               + RP(3,3);	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 	%                                                                    % 	%  AUTO-GENERATED EQUATIONS (S. Gros, HIGHWIND, OPTEC, KU Leuven)    % 	%                                                                    % 	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 	% Inverse of the Generalized inertia matrix 	%IntermediateState IMA(3,3); 	IMA(1,1) = 1/m; 	IMA(1,2) = 0; 	IMA(1,3) = 0; 	IMA(2,1) = 0; 	IMA(2,2) = 1/m; 	IMA(2,3) = 0; 	IMA(3,1) = 0; 	IMA(3,2) = 0; 	IMA(3,3) = 1/m; 	% G1 (right hand-side up) 	%IntermediateState G1(3,1); % 	G1(3,3) = F(1) + ddelta*dy*m + dddelta*m*y; % 	G1(3,2) = F(2) - (dddelta*m*(2*rA + 2*x))/2 - ddelta*dx*m; % 	G1(3,1) = F(3) - g*mgrav; 	G1(1,1) = F(1) + (m*(2*ddelta*dy + 2*ddelta*ddelta*rA + 2*ddelta*ddelta*x))/2 + ddelta*dy*m + dddelta*m*y; 	G1(2,1) = F(2) - (m*(2*ddelta*dx - 2*ddelta*ddelta*y))/2 - (dddelta*m*(2*rA + 2*x))/2 - ddelta*dx*m; 	G1(3,1) = F(3) - g*mgrav; 	% G2 (right hand-side down) 	%IntermediateState G2; 	G2 = - dx*dx - dy*dy - dz*dz; 	% NabG 	%IntermediateState NabG(3,1); 	NabG(1,1) = x; 	NabG(2,1) = y; 	NabG(3,1) = z; 	% NabGT 	%IntermediateState NabGT(1,3); 	NabGT(1,1) = x; 	NabGT(1,2) = y; 	NabGT(1,3) = z; 	% LambdaFac * lambda = lambdaright 	%IntermediateState LambdaFac; 	LambdaFac = NabGT*IMA*NabG; 	% lambdaright 	%IntermediateState lambdaright; 	lambdaright = NabGT*IMA*G1 - G2; 	% Consistency Conditions 	%IntermediateState Const, dConst; 	Const = - r*r/2 + x*x/2 + y*y/2 + z*z/2; 	dConst = dx*x + dy*y + dz*z; 	% lambda 	%IntermediateState lambda;     Pole = 1/2;    lambda = (2*Pole*dConst + Pole*Pole*Const + lambdaright)/LambdaFac; % 	lambda = lambdaright/LambdaFac; 	% ddq (accelerations) 	%IntermediateState ddq(3,1); 	ddq = IMA*(G1-NabG*lambda); 			% AIRCRAFT REF. FRAME ACCELERATION 	%IntermediateState ddxIMU; 	%IntermediateState ddyIMU; 	%IntermediateState ddzIMU; % 	%IntermediateState w1IMU; % 	%IntermediateState w2IMU; % 	%IntermediateState w3IMU; % 	ddxIMU = e11*ddq(1,1) + e12*ddq(2,1) + e13*(ddq(3,1) + g); % 	ddyIMU = e21*ddq(1,1) + e22*ddq(2,1) + e23*(ddq(3,1) + g); % 	ddzIMU = e31*ddq(1,1) + e32*ddq(2,1) + e33*(ddq(3,1) + g); %     % 	ddxIMU = e11*ddq(1,1) + e21*ddq(2,1) + e31*(ddq(3,1) + g); % 	ddyIMU = e12*ddq(1,1) + e22*ddq(2,1) + e32*(ddq(3,1) + g); % 	ddzIMU = e13*ddq(1,1) + e23*ddq(2,1) + e33*(ddq(3,1) + g); 	% 	%IntermediateState ddxAend, ddyAend; % 	ddxAend = ddelta*ddelta*rA*sin(delta);  % 	ddyAend = - rA*cos(delta)*ddelta*ddelta; % % 	ddxAend = ddxA;  % % 	ddyAend = ddyA; 		%IntermediateState G1end(3,1); % 	G1end(3,3) = F(1) + ddelta*dy*m; % 	G1end(3,2) = F(2) - ddelta*dx*m; % 	G1end(3,1) = F(3) - g*mgrav; 	G1end(3,3) = F(1) + (m*(2*ddelta*dy + 2*ddelta*ddelta*rA + 2*ddelta*ddelta*x))/2 + ddelta*dy*m; 	G1end(3,2) = F(2) - (m*(2*ddelta*dx - 2*ddelta*ddelta*y))/2                      - ddelta*dx*m; 	G1end(3,1) = F(3) - g*mgrav; 		% lambdaright 	%IntermediateState lambdarightend; 	lambdarightend = NabGT*IMA*G1end - G2; 	% lambda 	%IntermediateState lambdaend; 	lambdaend = lambdarightend/LambdaFac; 	% ddq (accelerations) 	%IntermediateState ddqend(3,1); 	ddqend = IMA*(G1end-NabG*lambdaend); 	%IntermediateState ddxIMUend; 	%IntermediateState ddyIMUend; 	%IntermediateState ddzIMUend; % 	ddxIMUend = e11*ddqend(3,3) + e12*ddqend(3,2) + e13*(ddqend(3,1) + g); % 	ddyIMUend = e21*ddqend(3,3) + e22*ddqend(3,2) + e23*(ddqend(3,1) + g); % 	ddzIMUend = e31*ddqend(3,3) + e32*ddqend(3,2) + e33*(ddqend(3,1) + g); %     % 	ddxIMUend = e11*ddqend(3,3) + e21*ddqend(3,2) + e31*(ddqend(3,1) + g); % 	ddyIMUend = e12*ddqend(3,3) + e22*ddqend(3,2) + e32*(ddqend(3,1) + g); % 	ddzIMUend = e13*ddqend(3,3) + e23*ddqend(3,2) + e33*(ddqend(3,1) + g); 			% 	ddyIMU = ddX(3,2)*(q0*q0 - q1*q1 + q2*q2 - q3*q3) + (ddX(3,1) + g)*(2*q0*q1 + 2*q2*q3) - ddX(3,3)*(2*q0*q3 - 2*q1*q2); % 	ddzIMU = ddX(3,3)*(2*q0*q2 + 2*q1*q3) - ddX(3,2)*(2*q0*q1 - 2*q2*q3) + (ddX(3,1) + g)*(q0*q0 - q1*q1 - q2*q2 + q3*q3); % 	w1IMU = w1; % 	w2IMU = w2; % 	w3IMU = w3;			%%%%%%%%%%%%%%/ END OF AUTO-GENERATED CODE %%%%%%%%%%%%%%%%%%%%%%%%%%% 		% TORQUES (BRIDLE)	% ---------------------------------------------------------------%     zT = -0.02;	TB(1) =  zT*(e12*lambda*x + e22*lambda*y + e32*lambda*z);	TB(2) = -zT*(e11*lambda*x + e21*lambda*y + e31*lambda*z);	TB(3) =  0;	%     TB = TB*0;    	% TORQUES (AERO)	% ---------------------------------------------------------------	T(1) =  0.5*rho*VKite2*SPAN*CR  + TB(1);	T(2) =  0.5*rho*VKite2*CHORD*CP + TB(2);	T(3) =  0.5*rho*VKite2*SPAN*CY  + TB(3);	%IntermediateState dw1, dw2, dw3;% 	dw1 = (I31*(T(3) + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3) - (I3*(T(1) - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3); % 	dw2 = (T(2) + w1*(I31*w1 + I3*w3) - w3*(I1*w1 + I31*w3))/I2; % 	dw3 = (I31*(T(1) - w2*(I31*w1 + I3*w3) + I2*w2*w3))/(I31*I31 - I1*I3) - (I1*(T(3) + w2*(I1*w1 + I31*w3) - I2*w1*w2))/(I31*I31 - I1*I3); 	    dw1 = (I31*T(3))/(I31*I31 - I1*I3) - (I3*T(1))/(I31*I31 - I1*I3);	dw2 = T(2)/I2;    dw3 = (I31*T(1))/(I31*I31 - I1*I3) - (I1*T(3))/(I31*I31 - I1*I3);        dw = [dw1;dw2;dw3];			%IntermediateState ConstR1, ConstR2, ConstR3, ConstR4, ConstR5, ConstR6;	ConstR1 = e11*e11 + e12*e12 + e13*e13 - 1;	ConstR2 = e11*e21 + e12*e22 + e13*e23;	ConstR3 = e11*e31 + e12*e32 + e13*e33;	ConstR4 = e21*e21 + e22*e22 + e23*e23 - 1;	ConstR5 = e21*e31 + e22*e32 + e23*e33;	ConstR6 = e31*e31 + e32*e32 + e33*e33 - 1;	switch flag	case ''	% System equation        sys =   [dx;dy;dz;ddq;de11;de12;de13;de21;de22;de23;de31;de32;de33;dw1;dw2;dw3;ddelta;dddelta;du1;du2];  	case 'output'                 sys = [FL+FD T CL ]';                case 'IMU'          % AIRCRAFT REF. FRAME ACCELERATION         ddx = ddq(1); ddy = ddq(2); ddz = ddq(3);         ddxIMU = ddx*e11 + ddy*e21 + ddz*e31 - ddelta^2*e11*x - ddelta^2*e21*y + 2*ddelta*dx*e21 - 2*ddelta*dy*e11 + dddelta*e21*rA + dddelta*e21*x - dddelta*e11*y - ddelta^2*e11*rA;        ddyIMU = ddx*e12 + ddy*e22 + ddz*e32 - ddelta^2*e12*x - ddelta^2*e22*y + 2*ddelta*dx*e22 - 2*ddelta*dy*e12 + dddelta*e22*rA + dddelta*e22*x - dddelta*e12*y - ddelta^2*e12*rA;        ddzIMU = ddx*e13 + ddy*e23 + ddz*e33 - ddelta^2*e13*x - ddelta^2*e23*y + 2*ddelta*dx*e23 - 2*ddelta*dy*e13 + dddelta*e23*rA + dddelta*e23*x - dddelta*e13*y - ddelta^2*e13*rA;	                RIMU = load('./IMU/RIMU.txt');        %         RIMU = eye(3);                aE = [ddxIMU;ddyIMU;ddzIMU];                 w = [w1;w2;w3];        %         aSHIFT = [XIMU3*dw2 - XIMU2*dw3 + w2*(XIMU2*w1 - XIMU1*w2) + w3*(XIMU3*w1 - XIMU1*w3);%                   XIMU1*dw3 - XIMU3*dw1 - w1*(XIMU2*w1 - XIMU1*w2) + w3*(XIMU3*w2 - XIMU2*w3);%                   XIMU2*dw1 - XIMU1*dw2 - w1*(XIMU3*w1 - XIMU1*w3) - w2*(XIMU3*w2 - XIMU2*w3)];                aIMU = RIMU*(aE);        wIMU = RIMU*w;                sys = [wIMU;aIMU;];              	case 'markers'                         % Markers' position in the body frame            pM1B = dlmread('CameraCalibration/pos_marker_body1.txt'); pM1B = pM1B(1:3);            pM2B = dlmread('CameraCalibration/pos_marker_body2.txt'); pM2B = pM2B(1:3);            pM3B = dlmread('CameraCalibration/pos_marker_body3.txt'); pM3B = pM3B(1:3);                        % Rotate and translate => absolute marker position            pM1 = [R*pM1B + [x;y;z]; 1];            pM2 = [R*pM2B + [x;y;z]; 1];            pM3 = [R*pM3B + [x;y;z]; 1];            pM = [pM1, pM2, pM3];                        % Transform the absolute position into pixels            RPC1 = dlmread('CameraCalibration/RPC1.txt');   % Transformation Matrix for camera 1            RPC2 = dlmread('CameraCalibration/RPC2.txt');   % Transformation Matrix for camera 2            PC1 = dlmread('CameraCalibration/PC1.txt');     % Projection Matrix for camera 1            PC2 = dlmread('CameraCalibration/PC2.txt');     % Projection Matrix for camera 2            %             PC1 = [pc1(1),      0, pc1(3);%                         0, pc1(2), pc1(4);%                         0,      0,      1];%             %             PC2 = [pc2(1),      0, pc2(3);%                         0, pc2(2), pc2(4);%                         0,      0,      1];            %             % Carousel rotation%             deltaFix = delta + pi/2;%             Rd   = [[cos(-deltaFix), -sin(-deltaFix),0,0];[sin(-deltaFix), cos(-deltaFix), 0,0];[0,0,1,0];[0,0,0,1]];%             %             % Anchorpoint translation%             TrAnch = eye(4);%             TrAnch(1,4) = [-rA];            %             keyboard            % Projection in the camera plane (pixels)            pRF1 = (RPC1)\pM;            uvsC1 = PC1*pRF1(1:3,:);            uvC1 = uvsC1(1:2,:)./repmat(uvsC1(3,:),2,1);            pRF2 = (RPC2)\pM;            uvsC2 = PC2*pRF2(1:3,:);            uvC2 = uvsC2(1:2,:)./repmat(uvsC2(3,:),2,1);                        %      pixels in camera1  oixels in camera2  absolute positions            sys = [reshape(uvC1,6,1); reshape(uvC2,6,1); pM1; pM2; pM3];                    case 'const'          ConstR1 = e11*e11 + e12*e12 + e13*e13 - 1;        ConstR2 = e11*e21 + e12*e22 + e13*e23;        ConstR3 = e11*e31 + e12*e32 + e13*e33;        ConstR4 = e21*e21 + e22*e22 + e23*e23 - 1;        ConstR5 = e21*e31 + e22*e32 + e23*e33;        ConstR6 = e31*e31 + e32*e32 + e33*e33 - 1;        sys = [Const;dConst;ConstR1;ConstR2;ConstR3;ConstR4;ConstR5;ConstR6;].';                    otherwise		error(['Unknown flag ''' flag '''.']);	end	