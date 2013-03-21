
// CONTROL :
// -------------------------
	Control             dddelta;  //  Carousel acceleration [rad/s^2]
	Control             dur;	  //  Aileron rate [(see above)/s]
	Control             dup;      //  Elevator rate [(see above)/s]
	
	// Collect all the controls in a vector
	const int n_U = 3; // Number of controls
	//IntermediateState U[n_U] = {dddelta, dur, dup};
