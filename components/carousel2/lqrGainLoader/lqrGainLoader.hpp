#ifndef __GAINLOADER__
#define __GAINLOADER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "LQRControllerGains.h"

class LqrGainLoader : public RTT::TaskContext
{
public:
	LqrGainLoader(std::string name);
	virtual ~LqrGainLoader(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::OutputPort< LQRControllerGains > portLQRGains;
private:

	LQRControllerGains gains;
};

#endif
