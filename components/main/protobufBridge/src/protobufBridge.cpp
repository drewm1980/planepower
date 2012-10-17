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
		: TaskContext(name)
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
		X.resize(NSTATES,0.0);
		return true;
	}

	bool  ProtobufBridge::startHook()
	{
		return true;
	}

	void  ProtobufBridge::updateHook()
	{
		_stateInputPort.read(X);
	}

	void  ProtobufBridge::stopHook()
	{
	}

	void  ProtobufBridge::cleanUpHook()
	{
	}

}//namespace

