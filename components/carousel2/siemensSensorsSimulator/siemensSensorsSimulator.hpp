#ifndef __SIEMENSSENSORSSIMULATOR__
#define __SIEMENSSENSORSSIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "SiemensDriveState.h"

/// Define the time-stamp type
typedef uint64_t TIME_TYPE;

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

protected:
	RTT::OutputPort< SiemensDriveState > portData;

private:
	SiemensDriveState state;
};

#endif
