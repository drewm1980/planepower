#ifndef LQR_CONTROLLER_DEBUG_H
#define LQR_CONTROLLER_DEBUG_H

// The gains for the LQR
// of our controller,
// plus a couple fields for timestamps
struct LQRControllerDebug
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

	double lqrTerm;
	double controlAsSpeed;

	double ts_trigger;
	double ts_elapsed;
};

#endif
