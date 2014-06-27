#ifndef RESAMPLED_MEASUREMENTS_H
#define RESAMPLED_MEASUREMENTS_H

// These are resampled and merged versions of the
// data from our various sensors.
// plus a couple fields for timestamps
struct ResampledMeasurements{
	double winchSpeedSmoothed;
	double winchEncoderPosition;
	double carouselSpeedSmoothed;
	double carouselEncoderPosition;
	//double winchTorque;
	//double winchPower;
	//double winchSpeedSetpoint;
	//double carouselTorque;
	//double carouselPower;
	//double carouselSpeedSetpoint;

	double azimuth;
	double elevation;

	double ts_trigger;
	double ts_elapsed;
};

#endif
