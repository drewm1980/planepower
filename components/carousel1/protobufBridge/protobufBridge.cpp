#include "protobufBridge.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT(ProtobufBridge);

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

ProtobufBridge::ProtobufBridge(std::string name):RTT::TaskContext(name,PreOperational)
{
	ports()->addEventPort("portTrigger",portTrigger).doc("This port triggers the protobuf");
	ports()->addPort("portLineAngleData", portLineAngleData).doc( "Line angle sensor data in radians" );
	ports()->addPort("portControls", portControls).doc( "Applied servo controls, in radians" );
}

ProtobufBridge::~ProtobufBridge()
{
}

bool  ProtobufBridge::configureHook()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	controls.resize(3, 0.0);

	context = new zmq::context_t(1);

	return true;
}

bool  ProtobufBridge::startHook()
{
	socket = new zmq::socket_t(*context,ZMQ_PUB);
	socket->bind("tcp://*:5563");

	return true;
}

void  ProtobufBridge::updateHook()
{
	portLineAngleData.read( lineAngleData );
	portControls.read( controls );

	sensors.mutable_lineangle()->set_hor(lineAngleData.angle_hor);
	sensors.mutable_lineangle()->set_ver(lineAngleData.angle_ver);

	sensors.mutable_servos()->set_right_aileron(controls[0]);
	sensors.mutable_servos()->set_left_aileron(controls[1]);
	sensors.mutable_servos()->set_elevator(controls[2]);
	
	if (!sensors.SerializeToString(&X_serialized)) {
		cerr << "Failed to serialize sensor." << endl;
		return;
	}
	s_sendmore(*socket, "yummy-sensordata");
	s_send(*socket, X_serialized);
}

void  ProtobufBridge::stopHook()
{
	delete socket;
}

void  ProtobufBridge::cleanUpHook()
{
	delete context;
	google::protobuf::ShutdownProtobufLibrary(); // optional
}

