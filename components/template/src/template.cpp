#include "simulateBall.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::SimulateBall)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 SimulateBall::SimulateBall(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

		// Add ports

		ports()->addPort( "inPort",_inPort ).doc("The description of the port");
		ports()->addPort( "outPort",_outPort ).doc("The description of the port");

		// Add operations

	provides()->addOperation("operationExample",&SimulateBall::simpleFunction,this).doc("Description of the method");
	}
	

	SimulateBall::~SimulateBall()
	{
	}

	bool  SimulateBall::configureHook()
	{
		return true;
	 }

	bool  SimulateBall::startHook()
	{
		return true;
	}

	void  SimulateBall::updateHook()
	{
	}

	void  SimulateBall::stopHook()
	{
	}

	void  SimulateBall::cleanUpHook()
	{
	}

	void SimulateBall::simpleFunction(){
	}


}//namespace

