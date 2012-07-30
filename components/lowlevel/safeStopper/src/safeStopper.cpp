#include "safeStopper.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::SafeStopper)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 SafeStopper::SafeStopper(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

		// Add ports

		ports()->addPort( "inPort",_inPort ).doc("The description of the port");
		ports()->addPort( "outPort",_outPort ).doc("The description of the port");

		// Add operations

	provides()->addOperation("operationExample",&SafeStopper::simpleFunction,this).doc("Description of the method");
	}
	

	SafeStopper::~SafeStopper()
	{
	}

	bool  SafeStopper::configureHook()
	{
		return true;
	 }

	bool  SafeStopper::startHook()
	{
		return true;
	}

	void  SafeStopper::updateHook()
	{
	}

	void  SafeStopper::stopHook()
	{
	}

	void  SafeStopper::cleanUpHook()
	{
	}

	void SafeStopper::simpleFunction(){
	}


}//namespace

