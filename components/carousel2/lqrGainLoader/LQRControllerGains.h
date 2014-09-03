#ifndef LQR_CONTROLLER_GAINS_H
#define LQR_CONTROLLER_GAINS_H

// The gains for the LQR controller,
// plus a couple fields for timestamps
struct LQRControllerGains
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

	// The two steady-state values we will
	// switch between
	double Xss0_0;
	double Xss0_1;
	double Xss0_2; // elevation
	double Xss0_3;
	double Xss0_4;
	double Xss0_5;
	double Xss0_6;
	double Xss0_7;
	double Xss0_8;

	double Xss1_0;
	double Xss1_1;
	double Xss1_2; // elevation
	double Xss1_3;
	double Xss1_4;
	double Xss1_5;
	double Xss1_6;
	double Xss1_7;
	double Xss1_8;

	double ts_trigger;
	double ts_elapsed;
};

#endif
