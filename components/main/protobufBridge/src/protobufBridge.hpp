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

#include <zmq.hpp>

// These files are in the "rawesome" git repo maintained by Greg Horn
#include "zhelpers.hpp"
#include "kite.pb.h"

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
			kite::Xyz xyz;
			kite::Dcm dcm;
			kite::CarouselState cs;

			//zmq::context_t context(1);
			//zmq::socket_t socket(context,ZMQ_PUB);
			zmq::context_t context;
			zmq::socket_t socket;

			string X_serialized;

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