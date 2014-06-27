#include <rtt/Service.hpp>

#include <rtt/types/GlobalsRepository.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/types/TypeInfoName.hpp>
#include <rtt/plugin/ServicePlugin.hpp>

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

/// A set of services that make our lives easier
class HighwindServices
	: public RTT::Service
{
public:
	HighwindServices(TaskContext* parent)
		: RTT::Service("HighwindServices", parent)
	{
		doc("A set of services that make our lives easier...");

		// add the operations
		addOperation("getTimestamp", &HighwindServices::getTimestamp, this)
			.doc( "Get the current timestamp." );
	}

	uint64_t getTimestamp( )
	{
		return TimeService::Instance()->getTicks();
	}
};

ORO_SERVICE_NAMED_PLUGIN(HighwindServices, "HighwindServices")
