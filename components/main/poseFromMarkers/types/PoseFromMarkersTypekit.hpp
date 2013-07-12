#ifndef __POSEFROMMARKERSTYPEKIT__
#define __POSEFROMMARKERSTYPEKIT__

#include "PoseFromMarkersDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/CArrayTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>
#include <rtt/types/BoostArrayTypeInfo.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, PoseFromMarkersDataType& l, unsigned int)
		{
			using boost::serialization::make_nvp;

			a & make_nvp("pose", l.pose);
						
			a & make_nvp("ts_trigger", l.ts_trigger);
			a & make_nvp("ts_entry", l.ts_entry);
			a & make_nvp("ts_elapsed", l.ts_elapsed);
		}
	}
}

struct PoseFromMarkersDataTypeInfo
	: public RTT::types::StructTypeInfo< PoseFromMarkersDataType >
{
	PoseFromMarkersDataTypeInfo()
		: RTT::types::StructTypeInfo< PoseFromMarkersDataType >( "PoseFromMarkersData" )
	{}
};

/// PoseFromMarkers typekit plugin
class PoseFromMarkersTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __POSEFROMMARKERSTYPEKIT__
