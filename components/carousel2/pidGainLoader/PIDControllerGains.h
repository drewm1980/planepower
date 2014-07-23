#ifndef PID_CONTROLLER_GAINS_H
#define PID_CONTROLLER_GAINS_H

// The gains for the PID
// of our controller,
// plus a couple fields for timestamps
struct PIDControllerGains
{
	// gain matrix
	double Kp;
	double Ki;
	double Kd;

	double ts_trigger;
	double ts_elapsed;
};

#endif
