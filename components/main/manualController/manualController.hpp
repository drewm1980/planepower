#ifndef __MANUALCONTROLLER__
#define __MANUALCONTROLLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <ocl/OCL.hpp>

#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

namespace OCL
{
	class ManualController
		: public TaskContext
	{
	protected:
		OutputPort<vector<double> >		_controlAction;
		vector<double>				controlAction;

	private:
		void					sendControl(double ur1, double ur2, double up);

	public:
		ManualController(std::string name);
		~ManualController();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
		
	};
}
#endif // __MANUALCONTROLLER__
