#ifndef __SIEMENSDRIVES__
#define __SIEMENSDRIVES__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class SiemensDrives : public RTT::TaskContext
{
public:
	SiemensDrives(std::string name);
	virtual ~SiemensDrives(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

};

#endif
