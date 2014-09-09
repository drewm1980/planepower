#ifndef __GAINLOADER__
#define __GAINLOADER__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

#include "PrecomputedData.h"

class LqrGainLoader : public RTT::TaskContext
{
public:
	LqrGainLoader(std::string name);
	virtual ~LqrGainLoader(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

protected:
	RTT::OutputPort< ControllerParameterization > portControllerParameterization;
	RTT::OutputPort< State > portxss;
	RTT::OutputPort< State > portxss0;
	RTT::OutputPort< State > portxss1;
	RTT::OutputPort< FeedbackMatrix > portG;
private:

	void reload_gains();
	PrecomputedData pd;
	
};

#endif
