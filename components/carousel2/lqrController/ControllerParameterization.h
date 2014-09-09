#pragma once

// The values we manually change to tune our controller
struct ControllerParameterization
{
	double R_control;
	double Q_alpha;
	double Q_dalpha;

	double ts_trigger;
	double ts_elapsed;
};

