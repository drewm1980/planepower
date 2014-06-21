#ifndef __CONTROLLERTEMPLATE__
#define __CONTROLLERTEMPLATE__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class ControllerTemplate : public RTT::TaskContext
{
public:
	ControllerTemplate(std::string name);
	virtual ~ControllerTemplate(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

};

#endif
