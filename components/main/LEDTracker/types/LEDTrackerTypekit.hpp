#ifndef __LEDTRACKERTYPEKIT__
#define __LEDTRACKERTYPEKIT__

#include "LEDTrackerDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, LEDTrackerDataType& l, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("positions", make_array(l.positions, 12));
			a & make_nvp("weights", make_array(l.weights, 12));
			
			a & make_nvp("ts_trigger", l.ts_trigger);
			a & make_nvp("ts_frame", l.ts_frame);
			a & make_nvp("ts_elapsed", l.ts_elapsed);
		}
	}
}

struct LEDTrackerDataTypeInfo
	: public RTT::types::StructTypeInfo< LEDTrackerDataType >
{
	LEDTrackerDataTypeInfo()
		: RTT::types::StructTypeInfo< LEDTrackerDataType >( "LEDTrackerData" )
	{}
};

/// LEDTracker typekit plugin
class LEDTrackerTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __LEDTRACKERTYPEKIT__
