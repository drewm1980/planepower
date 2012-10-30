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

		cs = mc.add_css(); 
		cs = mc.mutable_css(0);

		socket.bind("tcp://*:5563");

		return true;
	}

	bool  ProtobufBridge::startHook()
	{
		return true;
	}

	void ProtobufBridge::copy_to_protobuf(const StateVector *X , kite::CarouselState *cs)
	{
		kite::Xyz *xyz = cs->mutable_kitexyz();
		kite::Dcm *dcm = cs->mutable_kitedcm();
		xyz->set_x(X->x);
		xyz->set_y(X->y);
		xyz->set_z(X->z);
		dcm->set_r11(X->e.e11); // Note, there is an implicit transpose happening in here.
		dcm->set_r21(X->e.e12);
		dcm->set_r31(X->e.e13);
		dcm->set_r12(X->e.e21);
		dcm->set_r22(X->e.e22);
		dcm->set_r32(X->e.e23);
		dcm->set_r13(X->e.e31);
		dcm->set_r23(X->e.e32);
		dcm->set_r33(X->e.e33);
		cs->set_delta(X->delta);
	}

	void  ProtobufBridge::updateHook()
	{
		_stateInputPort.read(X);
		
		copy_to_protobuf((StateVector *) &X[0], cs);
		cs->set_rarm(1.085);
		cs->set_zt(-.03); // PROBABLY WRONG; AT LEAST ONLY FOR VISUALIZATION
		cs->set_w0(0);

		if (!mc.SerializeToString(&X_serialized)) {
			cerr << "Failed to serialize mc." << endl;
			return;
		}
		s_sendmore(socket, "multi-carousel");
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

