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
		ports()->addEventPort( "stateInputPort",_stateInputPort ).doc("x,y,z"
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

		xyz.set_x(X[0]);
		xyz.set_y(X[1]);
		xyz.set_z(X[2]);
		dcm.set_r11(X[6]);
		dcm.set_r21(X[7]);
		dcm.set_r31(X[8]);
		dcm.set_r12(X[9]);
		dcm.set_r22(X[10]);
		dcm.set_r32(X[11]);
		dcm.set_r13(X[12]);
		dcm.set_r23(X[13]);
		dcm.set_r33(X[14]);
		cs.mutable_kitexyz()->CopyFrom(xyz);
		cs.mutable_kitedcm()->CopyFrom(dcm);
		cs.set_delta(X[18]);
		cs.set_rarm(1.085);
		cs.set_zt(-.03);
		cs.set_w0(0);
		mc.mutable_css(0)->CopyFrom(cs);
		// To Add: Messages

		if (!mc.SerializeToString(&X_serialized)) {
			cerr << "Failed to serialize mc." << endl;
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

