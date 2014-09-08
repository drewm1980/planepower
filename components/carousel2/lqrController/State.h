#ifndef STATE_H
#define STATE_H

struct State
{
	// Our linearised state vector
	double delta_motor;
	double delta_arm;
	double alpha;
	double beta;
	double ddelta_motor;
	double ddelta_arm;
	double dalpha;
	double dbeta;
	double ddelta_motor_setpoint;

	double ts_trigger;
	double ts_elapsed;
};

#endif
