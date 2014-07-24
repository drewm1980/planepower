#ifndef PID_CONTROLLER_DEBUG_H
#define PID_CONTROLLER_DEBUG_H

// The gains for the PID
// of our controller,
// plus a couple fields for timestamps
struct PIDControllerDebug
{
	double Kp;
	double Ki;
	double Kd;

	double derivativeLowpassFilterState;

	double feedForwardTermAsAngle;
	double feedForwardTermAsSpeed;

	double ierror;
	double derror;

	double ts_trigger;
	double ts_elapsed;
};

#endif
