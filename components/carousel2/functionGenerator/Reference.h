#ifndef REFERENCE_H
#define REFERENCE_H

// The reference for the controller 
// and cycle clock for non obvious periodic signals
// plus a couple fields for timestamps
struct Reference 
{
	// Gain Matrix
	double elevation;
	double cycle;

	double ts_trigger;
	double ts_elapsed;
};

#endif
