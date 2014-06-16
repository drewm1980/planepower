#ifndef __SIEMENSSENSORS__
#define __SIEMENSSENSORS__

#include <stdint.h>

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include "siemens_communication.hpp"

class SiemensSensors : public RTT::TaskContext
{
public:
	SiemensSensors(std::string name);
	~SiemensSensors();

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

private:
	SiemensReceiver *receiver;
	SiemensDriveState *state;
};

#endif
