#ifndef __GAINLOADER__
#define __GAINLOADER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "ControllerGains.h"

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
	RTT::OutputPort< ControllerGains > portGains;
private:

	ControllerGains gains;
};

#endif
