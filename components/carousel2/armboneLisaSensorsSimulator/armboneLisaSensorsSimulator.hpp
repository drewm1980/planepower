#ifndef __ARMBONELISASENSORSSIMULATOR__
#define __ARMBONELISASENSORSSIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "armbone_lisa_communication.hpp"

class ArmboneLisaSensorsSimulator : public RTT::TaskContext
{
public:
	ArmboneLisaSensorsSimulator(std::string name);
	virtual ~ArmboneLisaSensorsSimulator(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();
protected:
//	RTT::InputPort< SiemensDriveState > portDriveState;
	RTT::OutputPort< ImuGyro > portGyroState;
	RTT::OutputPort< ImuMag > portMagState;
	RTT::OutputPort< ImuAccel > portAccelState;
private:
//	SiemensDriveState driveState;
	ImuGyro imuGyro;
	ImuMag imuMag;
	ImuAccel imuAccel;

};

#endif
