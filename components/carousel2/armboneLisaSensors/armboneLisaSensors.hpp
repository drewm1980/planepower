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
	RTT::OutputPort< ImuGyro > portGyroState;
	RTT::OutputPort< ImuMag > portMagState;
	RTT::OutputPort< ImuAccel > portAccelState;
private:
	ArmboneLisaReceiver receiver;
	ImuGyro imuGyro;
	ImuMag imuMag;
	ImuAccel imuAccel;
	bool keepRunning;
	
};

#endif
