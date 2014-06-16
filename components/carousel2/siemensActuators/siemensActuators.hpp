#ifndef __SIEMENSACTUATORS__
#define __SIEMENSACTUATORS__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>
#include "siemens_communication.hpp"

class SiemensActuators : public RTT::TaskContext
{
public:
	SiemensActuators(std::string name);
	virtual ~SiemensActuators();

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

private:
	SiemensSender *sender;
};

#endif
