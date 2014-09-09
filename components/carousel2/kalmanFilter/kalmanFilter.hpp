#ifndef __KALMANFILTER__
#define __KALMANFILTER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "ResampledMeasurements.h"
#include "SiemensDriveCommand.h"

#include "Reference.h"
#include "LQRGains.h"

typedef uint64_t TIME_TYPE;

class KalmanFilter : public RTT::TaskContext
{
public:
	KalmanFilter(std::string name);
	virtual ~KalmanFilter(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	// This Kalman Filter operates purely from line angle measurements.
	RTT::InputPort< LineAngles > portLineAngles;
	RTT::InputPort< State > portReference;

	RTT::InputPort< LQRGains > portGainsIn;
	RTT::OutputPort< LQRGains > portGainsOut;

	RTT::OutputPort< State > portStateEstimate;

private:
	LineAngles lineAngles;
	LQRGains gains;
	Reference reference;

	State stateEstimate;

	State highReference;
	State lowReference;

	TIME_TYPE trigger_last;	
	TIME_TYPE trigger;

	LQRControllerDebug debug;
};

#endif
