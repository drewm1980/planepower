#ifndef __GAINLOADER__
#define __GAINLOADER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class GainLoader : public RTT::TaskContext
{
public:
	GainLoader(std::string name);
	virtual ~GainLoader(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();
protected:
//	RTT::InputPort< SiemensDriveState > portDriveState;
//	RTT::InputPort< LineAngles > portLineAngles;
//	RTT::OutputPort< ResampledMeasurements > portData;
private:
//	SiemensDriveState driveState;
//	LineAngles lineAngles;
//	ResampledMeasurements resampledMeasurements;

};

#endif
