#ifndef __GAINLOADER__
#define __GAINLOADER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "LQRGains.h"
#include "State.h"

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
	RTT::OutputPort< LQRGains > portLQRGains;
	RTT::OutputPort< State > portxss0;
	RTT::OutputPort< State > portxss1;
private:

	LQRGains gains;
	State stateHolder; // A temporary struct for loading properties
	State xss, xss0, xss1;
};

#endif
