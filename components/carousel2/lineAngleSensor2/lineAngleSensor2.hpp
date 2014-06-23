#ifndef __LINEANGLESENSOR2__
#define __LINEANGLESENSOR2__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class LineAngleSensor2 : public RTT::TaskContext
{
public:
	LineAngleSensor2(std::string name);
	virtual ~LineAngleSensor2(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

private:
	bool keepRunning;
};

#endif
