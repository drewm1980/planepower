#include "controlDerivativeHandler.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ControlDerivativeHandler)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 ControlDerivativeHandler::ControlDerivativeHandler(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		addProperty( "Nintervals",Nintervals ).doc("The number of intervals for approximating the slope");
		addProperty( "Period",Period ).doc("The period in seconds");
		Nintervals = 2;
		Period = 0.1;
		// Add ports

		addEventPort("portControls", portControls,boost::bind(&ControlDerivativeHandler::startControl,this)).doc("MPC controls, with derivatives:aileron1, aileron2, elevator, d aileron1/dt, d aileron2/dt, delevator/dt");
		addPort( "portControlsOut",portControlsOut ).doc("The controls to be sent to the mcu: aileron1, aileron2, elevator");
		ControlsOut.resize(3);

	}
	

	ControlDerivativeHandler::~ControlDerivativeHandler()
	{
	}

	bool  ControlDerivativeHandler::configureHook()
	{
		return true;
	 }

	bool  ControlDerivativeHandler::startHook()
	{
		return true;
	}

	void  ControlDerivativeHandler::updateHook()
	{
	}

	void  ControlDerivativeHandler::stopHook()
	{
	}

	void  ControlDerivativeHandler::cleanUpHook()
	{
	}

	void ControlDerivativeHandler::startControl()
	{
		if(portControls.read(Controls)!=NewData){
			return;
		}
		double dt = (double) Period/Nintervals;
		ControlsOut[0] = Controls[0] + dt*Controls[3];
		ControlsOut[1] = Controls[1] + dt*Controls[4];
		ControlsOut[2] = Controls[2] + dt*Controls[5];
		portControlsOut.write(ControlsOut);
		for(int i=1; i<Nintervals; i++){
			usleep((int) dt*1e6);
			ControlsOut[0] = Controls[0] + i*dt*Controls[3];
			ControlsOut[1] = Controls[1] + i*dt*Controls[4];
			ControlsOut[2] = Controls[2] + i*dt*Controls[5];
			portControlsOut.write(ControlsOut);
		}

		
	}


}//namespace

