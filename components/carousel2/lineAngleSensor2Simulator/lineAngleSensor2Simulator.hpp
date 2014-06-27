#ifndef __LINEANGLESENSOR2SIMULATOR__
#define __LINEANGLESENSOR2SIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "LineAngles.h"

class LineAngleSensor2Simulator : public RTT::TaskContext
{
public:
	LineAngleSensor2Simulator(std::string name);
	virtual ~LineAngleSensor2Simulator(){};

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

};

#endif
