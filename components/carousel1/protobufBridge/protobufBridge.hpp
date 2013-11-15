#ifndef __PROTOBUFBRIDGE__
#define __PROTOBUFBRIDGE__

#include <fstream>
using std::ifstream;

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <ocl/OCL.hpp>

#include "LineAngleSensorDataType.hpp"
#include <zmq.hpp>

#include "zhelpers.hpp"
#include "sensors.pb.h"

using namespace std;
using namespace RTT;

typedef uint64_t TIME_TYPE;

class ProtobufBridge : public TaskContext
{
	protected:
		InputPort< LineAngleSensorDataType > portLineAngleData;
		LineAngleSensorDataType lineAngleData;

		InputPort< TIME_TYPE > portTrigger;

		InputPort< std::vector< double > > portControls;
		std::vector< double > controls;

	private:
		zmq::context_t *context;
		zmq::socket_t *socket;

		sensors::Sensors sensors;

		string X_serialized;

	public:
		ProtobufBridge(std::string name);
		~ProtobufBridge();
		bool configureHook();
		bool startHook();
		void updateHook();
		void stopHook();
		void cleanUpHook();

};
#endif 
