#ifndef __SIEMENSACTUATORSSIMULATOR__
#define __SIEMENSACTUATORSSIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "SiemensDriveCommand.h"

class SiemensActuatorsSimulator : public RTT::TaskContext
{
public:
	SiemensActuatorsSimulator(std::string name);
	virtual ~SiemensActuatorsSimulator(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::InputPort< SiemensDriveCommand > portControls;
	SiemensDriveCommand driveCommand;

};

#endif
