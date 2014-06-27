#ifndef CONTROLLER_GAINS_H
#define CONTROLLER_GAINS_H

// The gain matrix (and/or other parameterization)
// of our controller,
// plus a couple fields for timestamps
struct ControllerGains
{
	// Gain Matrix
	double k11, k12;
	double k21, k22;

	double ts_trigger;
	double ts_elapsed;
};

#endif
