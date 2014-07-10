#ifndef __ARMBONELISASENSORS__
#define __ARMBONELISASENSORS__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "armbone_lisa_communication.hpp"

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
	RTT::OutputPort< ImuGyro > portGyroState;
	RTT::OutputPort< ImuMag > portMagState;
	RTT::OutputPort< ImuAccel > portAccelState;
private:
//	SiemensDriveState driveState;
//	LineAngles lineAngles;
//	ResampledMeasurements resampledMeasurements;
	ArmboneLisaReceiver receiver;
	ImuGyro imuGyro;
	ImuMag imuMag;
	ImuAccel imuAccel;
	bool keepRunning;
	
};

#endif
