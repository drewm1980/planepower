#include "protobufBridge.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ProtobufBridge)

	using namespace std;
	using namespace RTT;
	using namespace Orocos;
	using namespace BFL;

	namespace OCL
{
	ProtobufBridge::ProtobufBridge(std::string name)
		: TaskContext(name) , context(1) , socket(context, ZMQ_PUB)
	{
		ports()->addPort( "stateInputPort",_stateInputPort ).doc("x,y,z"
					 ",dx,dy,dz"
					 ",e11,e12,e13,e21,e22,e23,e31,e32,e33"
					 ",w1,w2,w3"
					 ",delta,ddelta,ur,up"); // We ONLY use the first 18 states from this;
											 // mismatch in u has no effect on us.
	}

	ProtobufBridge::~ProtobufBridge()
	{
	}

	bool  ProtobufBridge::configureHook()
	{
		GOOGLE_PROTOBUF_VERIFY_VERSION;

		X.resize(NSTATES,0.0);

		socket.bind("tcp://*:5563");

		return true;
	}

	bool  ProtobufBridge::startHook()
	{
		return true;
	}

	void  ProtobufBridge::updateHook()
	{
		_stateInputPort.read(X);

		xyz.set_x(2);
		xyz.set_y(2);
		xyz.set_z(2);
		dcm.set_r11(1);
		dcm.set_r12(0);
		dcm.set_r13(0);
		dcm.set_r21(0);
		dcm.set_r22(1);
		dcm.set_r23(0);
		dcm.set_r31(0);
		dcm.set_r32(0);
		dcm.set_r33(1);
		cs.mutable_kitexyz()->CopyFrom(xyz);
		cs.mutable_kitedcm()->CopyFrom(dcm);
		cs.set_delta(0.1);
		cs.set_rarm(1);
		cs.set_zt(-0.5);
		// To Add: Messages
		cs.set_w0(0);

		if (!cs.SerializeToString(&X_serialized)) {
			cerr << "Failed to serialize cs." << endl;
			return;
		}
		s_sendmore(socket, "carousel");
		s_send(socket, X_serialized);
	}

	void  ProtobufBridge::stopHook()
	{
	}

	void  ProtobufBridge::cleanUpHook()
	{
		google::protobuf::ShutdownProtobufLibrary(); // optional
	}

}//namespace

