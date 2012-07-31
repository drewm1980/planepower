#ifndef __CONTROLDERIVATIVEHANDLER__
#define __CONTROLDERIVATIVEHANDLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <ocl/OCL.hpp>

#include <unistd.h>

#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

namespace OCL
{

	/// ControlDerivativeHandler class
	/**
	This class simulates the free motion of a ball attached to a pendulum.
	The pendulum motion is executed in the x=0 plane of the pendulum reference
	frame. The state of the ball in the pendulum plane is given by
	[theta,omega,alpha]. The position of the ball in the world frame is given by
	[x,y,z].
	The pendulum reference frame wrt to the world reference frame gives the pose
	of the pendulum motion plane wrt to the world.
	*/
	class ControlDerivativeHandler
		: public TaskContext
	{
	protected:
		/*********
		DATAPORTS
		*********/
		//! Input port
		InputPort<vector<double> >		portControls;
		//! Output port
		OutputPort<vector<double> >		portControlsOut;

	private:
		void					startControl(void);
		vector<double>				Controls;
		vector<double>				ControlsOut;
		int					Nintervals;
		double					Period;

	public:
		ControlDerivativeHandler(std::string name);
		~ControlDerivativeHandler();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
		
	};
}
#endif // __CONTROLDERIVATIVEHANDLER__
