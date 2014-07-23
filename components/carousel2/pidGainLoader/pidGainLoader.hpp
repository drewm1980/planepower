#ifndef __GAINLOADER__
#define __GAINLOADER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "PIDControllerGains.h"

class PidGainLoader : public RTT::TaskContext
{
public:
	PidGainLoader(std::string name);
	virtual ~PidGainLoader(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::OutputPort< PIDControllerGains > portPIDGains;
private:

	PIDControllerGains gains;
};

#endif
