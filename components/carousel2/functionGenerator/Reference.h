#ifndef REFERENCE_H
#define REFERENCE_H

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
