                                     %  PARAMETERS OF THE KITE :
                                     %  -----------------------------
            mk =  0.463;      %  mass of the kite               %  [ kg    ]
           %  A =  0.2;      %  effective area                 %  [ m^2   ]
         
         
                                     %   PHYSICAL CONSTANTS :
                                     %  -----------------------------
             g =    9.81;      %  gravitational constant         %  [ m /s^2]
           rho =    1.23;      %  density of the air             %  [ kg/m^3]

                                     %  PARAMETERS OF THE CABLE :
                                     %  -----------------------------
          rhoc = 1450.00;      %  density of the cable           %  [ kg/m^3]
            cc =   1.00;      %  frictional constant            %  [       ]
            dc = 1e-3;      %  diameter                       %  [ m     ]

        
            
            AQ      =  pi*dc^2/4.0;
           
            InputFac = 1e-3;%(1/1150)/(20/32767)*(pi/180);
            
            %CAROUSEL ARM LENGTH
            rA = 1.085; %(dixit Kurt)
            
           
            %OFFSET OF THE IMU POSITION
%             xIMU = 2*5e-2;
%            % yIMU = 5e-2;
%             zIMU = 0e-2;
%             alphaIMU = 0;
            
            %INERTIA MATRIX (Kurt's direct measurements)
            I1 = 0.0163;I31 = 0.0006;
            I2 = 0.0078;
            I3 = 0.0229;
            
            
            %TAIL LENGTH
            LT = 0.45;
       
            
            %ROLL DAMPING
            RD = 1e-2; %TO BE IDENTIFIED
            
           %WIND-TUNNEL PARAMETERS
           
           %Lift (report p. 67)
           %Trim = 0*pi/180; %TRIMMING, UNKNOWN !!
           CLA = 5.064;
           CLB = 0;
           CLe = 0.318;
           CLr = 0.85; %?!?!?!?!?
           CL0 = 0.239;
           
           %Drag (report p. 70)
           CDA = -0.195;
           CDA2 = 4.268;
           CDB2 = 0;
           CDe = 0.044;
           CDr = 0.111;
           CD0 = 0.026;
           
           %Roll (report p. 72)
           CRB = -0.062;
           CRAB = -0.271;
           CRr = -0.244;
           
           %Pitch (report p. 74)
           CPA = 0.293;
           CPe = -0.821;
           CPr = -0.647; %?!?!?!?!?
           CP0 = 0.03;
           
           %Yaw (report p. 76)
           CYB = 0.05;
           CYAB = 0.229;
           
           SPAN = 0.96;
           CHORD = 0.1;
           
           
           
           
