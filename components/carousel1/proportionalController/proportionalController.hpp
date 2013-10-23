#ifndef __PROPORTIONALCONTROLLER__
#define __PROPORTIONALCONTROLLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class ProportionalController : public RTT::TaskContext
{
public:
	ProportionalController(std::string name);
	virtual ~ProportionalController(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

};

#endif
