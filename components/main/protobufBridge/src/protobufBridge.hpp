#ifndef __PROTOBUFBRIDGE__
#define __PROTOBUFBRIDGE__

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

#define NSTATES 22

namespace OCL
{
	class ProtobufBridge
		: public TaskContext
	{
		protected:
			InputPort<vector<double> >			_stateInputPort;
			vector<double>						X;

		private:

		public:
			ProtobufBridge(std::string name);
			~ProtobufBridge();
			bool		configureHook();
			bool		startHook();
			void		updateHook();
			void		stopHook();
			void		cleanUpHook();
	};
}
#endif // __PROTOBUFBRIDGE__
