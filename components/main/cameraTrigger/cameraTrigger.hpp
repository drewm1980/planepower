#ifndef __CAMERATRIGGER__
#define __CAMERATRIGGER__

#include <stdint.h>
#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <rtt/os/TimeService.hpp>

#include <ocl/OCL.hpp>

#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

typedef uint64_t TIME_TYPE;

#define TRIGGER_ACTIVE_HIGH 0

class CameraTrigger
: public TaskContext
{
	protected:
		OperationCaller<bool(unsigned int, bool)>setBit;
		void pull_trigger_high();
		void pull_trigger_low();
		InputPort<TIME_TYPE>	_Trigger;
		OutputPort<TIME_TYPE>	_TriggerTriggeredTime; // Mainly for debugging
		OutputPort<TIME_TYPE>	_TriggerResetTime; // Mainly for debugging
	private:
		TIME_TYPE tempTime;

	public:
		CameraTrigger(std::string name);
		~CameraTrigger();
		bool        configureHook();
		bool        startHook();
		void        updateHook();
		void        stopHook();
		void        cleanUpHook();
};

#endif
