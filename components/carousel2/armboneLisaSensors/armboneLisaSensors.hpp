#ifndef __ARMBONELISASENSORS__
#define __ARMBONELISASENSORS__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class ArmboneLisaSensors : public RTT::TaskContext
{
public:
	ArmboneLisaSensors(std::string name);
	virtual ~ArmboneLisaSensors(){};

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
