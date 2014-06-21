#ifndef __RESAMPLER__
#define __RESAMPLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class Resampler : public RTT::TaskContext
{
public:
	Resampler(std::string name);
	virtual ~Resampler(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

};

#endif
