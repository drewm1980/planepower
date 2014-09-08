#ifndef LQR_GAINS_H
#define LQR_GAINS_H

// The gains for the LQR controller,
// plus a couple fields for timestamps
struct LQRGains
{
	// These are the values that were
	// used to derive the ~actual gain matrix
	double R_control;
	double Q_alpha; // elevation
	double Q_dalpha; // d/dt elevation

	// The actual gain matrix
	double K0;
	double K1;
	double K2;
	double K3;
	double K4;
	double K5;
	double K6;
	double K7;
	double K8;

	double ts_trigger;
	double ts_elapsed;
};

#define STATECOUNT 9

#endif
