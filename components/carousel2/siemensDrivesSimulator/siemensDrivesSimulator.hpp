#ifndef __SIEMENSDRIVESSIMULATOR__
#define __SIEMENSDRIVESSIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class SiemensDrivesSimulator : public RTT::TaskContext
{
public:
	SiemensDrivesSimulator(std::string name);
	virtual ~SiemensDrivesSimulator(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

};

#endif
