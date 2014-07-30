#ifndef PID_CONTROLLER_DEBUG_H
#define PID_CONTROLLER_DEBUG_H

// The gains for the PID
// of our controller,
// plus a couple fields for timestamps
struct PIDControllerDebug
{
	double referenceElevation;
	double referenceSpeed;
	double unfilteredReferenceElevation;
	double unfilteredReferenceSpeed;
	double elevation;

	double Kp;
	double Ki;
	double Kd;

	double derivativeLowpassFilterState;
	double d_elevation;
	double dt;

	double error;
	double ierror;
	double derror;

	double feedForwardTermAsAngle;
	double feedForwardTermAsSpeed;

	double pTerm;
	double iTerm;
	double dTerm;

	double pidTerm;
	double controlAsSpeed;

	double ts_trigger;
	double ts_elapsed;
};

#endif
