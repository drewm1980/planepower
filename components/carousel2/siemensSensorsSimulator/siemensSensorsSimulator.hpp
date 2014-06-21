#ifndef __SIEMENSSENSORSSIMULATOR__
#define __SIEMENSSENSORSSIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class SiemensSensorsSimulator : public RTT::TaskContext
{
public:
	SiemensSensorsSimulator(std::string name);
	virtual ~SiemensSensorsSimulator(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

};

#endif
