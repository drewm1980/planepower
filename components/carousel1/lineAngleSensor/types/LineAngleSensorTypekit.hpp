#ifndef __LINEANGLESENSORTYPEKIT__
#define __LINEANGLESENSORTYPEKIT__

#include "LineAngleSensorDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, LineAngleSensorDataType& l, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("angle_hor", l.angle_hor);
			a & make_nvp("angle_ver", l.angle_ver);
			
			a & make_nvp("ts_trigger", l.ts_trigger);
			a & make_nvp("ts_elapsed", l.ts_elapsed);
		}
	}
}

struct LineAngleSensorDataTypeInfo
	: public RTT::types::StructTypeInfo< LineAngleSensorDataType >
{
	LineAngleSensorDataTypeInfo()
		: RTT::types::StructTypeInfo< LineAngleSensorDataType >( "LineAngleSensorData" )
	{}
};

/// LineAngleSensor typekit plugin
class LineAngleSensorTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __LINEANGLESENSORTYPEKIT__
