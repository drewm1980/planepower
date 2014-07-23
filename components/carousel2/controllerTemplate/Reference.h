#ifndef CONTROLLER_GAINS_H
#define CONTROLLER_GAINS_H

// The gain matrix (and/or other parameterization)
// of our controller,
// plus a couple fields for timestamps
struct Reference 
{
	// Gain Matrix
	double elevation;


	double ts_trigger;
	double ts_elapsed;
};

#endif
