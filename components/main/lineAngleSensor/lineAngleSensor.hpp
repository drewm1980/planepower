#ifndef __LINE_ANGLE_SENSOR__
#define __LINE_ANGLE_SENSOR__

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
using namespace Orocos;

typedef uint64_t TIME_TYPE;

class LineAngleSensor
: public TaskContext
{
	protected:
		OperationCaller<double(unsigned int)>readAnalog;
		InputPort<TIME_TYPE>	_TriggerIn;
		OutputPort<TIME_TYPE>	_TriggerOut;
		OutputPort<TIME_TYPE>	_EboxRequestTime;
		OutputPort<TIME_TYPE>	_EboxReplyTime;
		OutputPort<vector<double> >		_voltages;

	private:
		TIME_TYPE tempTime;
		vector<double> voltages;

	public:
		LineAngleSensor(std::string name);
		~LineAngleSensor();
		bool        configureHook();
		bool        startHook();
		void        updateHook();
		void        stopHook();
		void        cleanUpHook();
};

#endif
