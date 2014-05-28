#ifndef __LINEANGLESENSOR2__
#define __LINEANGLESENSOR2__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "LineAngles.h"

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

protected:
	RTT::OutputPort< LineAngles > portData;
private:
	LineAngles lineAngles;
	bool keepRunning;
};

#endif
