#ifndef __SEGFAULTER__
#define __SEGFAULTER__

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
	class SegFaulter
		: public TaskContext
	{
	protected:
	private:
		int countDown;
	public:
		SegFaulter(std::string name);
		~SegFaulter();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
	};
}
#endif // __SEGFAULTER__
