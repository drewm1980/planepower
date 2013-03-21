// DIFFERENTIAL STATES :
// -------------------------
  
	DifferentialState      x;      // Position [m]
	DifferentialState	   y;      //  
	DifferentialState      z;      //  
// -------------------------      //  -------------------------------------------
  
	DifferentialState     dx;      //  Speed [m/s]
	DifferentialState     dy;      //  
	DifferentialState     dz;      //  
	
	// 3x3 Rotation matrix e maps vector in aircraft frame back to arm tip frame. 
	// e12 is row 1, column 2 of e. (row major):
	// e = [e11 e12 e13
	//      e21 e22 e23
	//      e31 e32 e33]
	DifferentialState    e11; 
	DifferentialState    e12;
	DifferentialState    e13;
	DifferentialState    e21;
	DifferentialState    e22;
	DifferentialState    e23;
	DifferentialState    e31;
	DifferentialState    e32;
	DifferentialState    e33;
	
	// Rotational velocity of the plane relative to arm end frame, expressed in aircraft frame
	DifferentialState    w1;
	DifferentialState    w2;
	DifferentialState    w3;	

	DifferentialState  delta;      //  Carousel angle, counter-clockwise when viewed from above [rad]
	DifferentialState ddelta;      //  Carousel rotation angle rate [rad/s]

	// Units are servo command values ranging in [-3.2767 3.2767].  = +/- 2^15/10^4
	// They get scaled by 10^4 and cast to an integer before getting sent to the servos
	DifferentialState     ur;	   //  Roll control surface input.  Positive value rolls plane left from pilot's perspective.
	DifferentialState     up;      //  Elevator control surface input.    Positive value pitches plane up from pilot's perpective.
	
	// Collect all the states in a vector
	const int n_XD = 22; // Number of states
	//IntermediateState XD[n_XD] = {x, y, z, dx, dy, dz, e11, e12, e13, e21, e22, e23, e31, e32, e33, w1, w2, w3, delta, ddelta, ur, up};
