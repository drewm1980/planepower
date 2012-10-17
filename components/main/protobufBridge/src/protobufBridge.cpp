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
		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

		// Add ports

		ports()->addPort( "inPort",_inPort ).doc("The description of the port");
		ports()->addPort( "outPort",_outPort ).doc("The description of the port");

		// Add operations

	provides()->addOperation("operationExample",&ProtobufBridge::simpleFunction,this).doc("Description of the method");
	}
	

	ProtobufBridge::~ProtobufBridge()
	{
	}

	bool  ProtobufBridge::configureHook()
	{
		return true;
	 }

	bool  ProtobufBridge::startHook()
	{
		return true;
	}

	void  ProtobufBridge::updateHook()
	{
	}

	void  ProtobufBridge::stopHook()
	{
	}

	void  ProtobufBridge::cleanUpHook()
	{
	}

	void ProtobufBridge::simpleFunction(){
	}


}//namespace

