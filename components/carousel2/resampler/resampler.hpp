#ifndef __RESAMPLER__
#define __RESAMPLER__

#include <stdint.h>

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include "LineAngles.h"
#include "SiemensDriveState.h"
#include "ResampledMeasurements.h"

class Resampler : public RTT::TaskContext
{
public:
	Resampler(std::string name);
	virtual ~Resampler(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::InputPort< SiemensDriveState > portDriveState;
	RTT::InputPort< LineAngles > portLineAngles;
	RTT::OutputPort< ResampledMeasurements > portData;
	
private:
	SiemensDriveState driveState;
	LineAngles lineAngles;
	ResampledMeasurements resampledMeasurements;

};

#endif
