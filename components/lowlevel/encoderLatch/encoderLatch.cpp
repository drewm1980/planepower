#include "encoderLatch.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::EncoderLatch)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 EncoderLatch::EncoderLatch(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

		// Add ports
		addEventPort( "triggerTimeStampIn",_triggerTimeStampIn );
		addPort( "triggerTimeStampOut",_triggerTimeStampOut );
		ports()->addPort( "deltaOmegaIn",_deltaOmegaIn ).doc("The description of the port");
		ports()->addPort( "deltaOmega",_deltaOmegaOut ).doc("The description of the port");

		// Add operations

	provides()->addOperation("operationExample",&EncoderLatch::simpleFunction,this).doc("Description of the method");
	}
	

	EncoderLatch::~EncoderLatch()
	{
	}

	bool  EncoderLatch::configureHook()
	{
		return true;
	 }

	bool  EncoderLatch::startHook()
	{
		return true;
	}

	void  EncoderLatch::updateHook()
	{
		_deltaOmegaIn.read(deltaOmega);
		_triggerTimeStampIn.read(triggerTimeStamp);
		_deltaOmegaOut.write(deltaOmega);
		_triggerTimeStampOut.write(triggerTimeStamp);
	}

	void  EncoderLatch::stopHook()
	{
	}

	void  EncoderLatch::cleanUpHook()
	{
	}

	void EncoderLatch::simpleFunction(){
	}


}//namespace

