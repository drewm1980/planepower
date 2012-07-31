#include "imuSimulator.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ImuSimulator)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
     ImuSimulator::ImuSimulator(std::string name)
         : TaskContext(name)
     {
        log(Debug) << "(ImuSimulator) constructor entered" << endlog();

        // Add properties

        properties()->addProperty( "prop",_prop ).doc("The description of the property");

	// Add ports

        ports()->addPort( "inPort",_inPort ).doc("The description of the port");
        ports()->addPort( "outPort",_outPort ).doc("The description of the port");

	// Add operations

	provides()->addOperation("operationExample",&ImuSimulator::simpleFunction,this).doc("Description of the method");

        log(Debug) << "(ImuSimulator) constructor finished" << endlog();
    }
    

    ImuSimulator::~ImuSimulator()
    {
    }

    bool  ImuSimulator::configureHook()
    {
        log(Debug) << "(ImuSimulator) ConfigureHook entered" << endlog();

        //Put here code that is needed for configuration of the component

        log(Debug) << "(ImuSimulator) ConfigureHook finished" << endlog();
        
        return true;
     }

    bool  ImuSimulator::startHook()
    {
        log(Debug) << "(ImuSimulator) startHook entered" << endlog();

        //Put here code that is needed to start the component

        log(Debug) << "(ImuSimulator) startHook finished" << endlog();
        return true;
    }

    void  ImuSimulator::updateHook()
    {
        log(Debug) << "(ImuSimulator) updateHook entered" << endlog();
	
	//Put here code that is needed for the updating of the component

        log(Debug) << "(ImuSimulator) updateHook finished" << endlog();
    }

    void  ImuSimulator::stopHook()
    {
        log(Debug) << "(ImuSimulator) stopHook entered" << endlog();

        //Put here code that is needed to stop the component

        log(Debug) << "(ImuSimulator) stopHook finished" << endlog();
    }

    void  ImuSimulator::cleanUpHook()
    {
        log(Debug) << "(ImuSimulator) cleanupHook entered" << endlog();

        //Put here code that is needed to clean up the component

        log(Debug) << "(ImuSimulator) cleanupHook finished" << endlog();
    }

    void ImuSimulator::simpleFunction(){
	// just an empty function
    }


}//namespace

