#ifndef __SIEMENSSENSORS__
#define __SIEMENSSENSORS__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class SiemensSensors : public RTT::TaskContext
{
public:
	SiemensSensors(std::string name);
	virtual ~SiemensSensors(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

private:
	SiemensReceiver receiver;
	SiemensDriveState state;
};

#endif
