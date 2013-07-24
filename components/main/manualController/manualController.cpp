#include "manualController.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ManualController)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 ManualController::ManualController(std::string name)
		 : TaskContext(name)
	 {
		ports()->addPort( "controlAction",_controlAction ).doc("controlAction");

		provides()->addOperation("sendControl",&ManualController::sendControl,this).doc("Description of the method");

		controlAction.resize(3);
	}

	ManualController::~ManualController()
	{
	}

	bool  ManualController::configureHook()
	{
		return true;
	 }

	bool  ManualController::startHook()
	{
		controlAction[0] = 0.0;
		controlAction[1] = 0.0;
		controlAction[2] = 0.0;
		_controlAction.write(controlAction);
		return true;
	}

	void  ManualController::updateHook()
	{
	}

	void  ManualController::stopHook()
	{
		controlAction[0] = 0.0;
		controlAction[1] = 0.0;
		controlAction[2] = 0.0;
		_controlAction.write(controlAction);
	}

	void  ManualController::cleanUpHook()
	{
	}

	void ManualController::sendControl(double ur1, double ur2, double up){
		controlAction[0] = ur1;
		controlAction[1] = ur2;
		controlAction[2] = up;
		_controlAction.write(controlAction);
	}


}//namespace

