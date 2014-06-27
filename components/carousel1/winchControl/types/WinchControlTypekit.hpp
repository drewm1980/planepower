#ifndef __WINCH_CONTROL_TYPEKIT__
#define __WINCH_CONTROL_TYPEKIT__

#include "WinchControlDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, WinchControlDataType& e, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("length", e.length);
			a & make_nvp("speed", e.speed);

			a & make_nvp("ts_trigger", e.ts_trigger);
			a & make_nvp("ts_elapsed", e.ts_elapsed);

			a & make_nvp("dbg_theta", e.dbg_theta);
			a & make_nvp("dbg_omega", e.dbg_omega);
			a & make_nvp("dbg_current", e.dbg_current);
		}
	}
}

struct WinchControlDataTypeInfo
	: public RTT::types::StructTypeInfo< WinchControlDataType >
{
	WinchControlDataTypeInfo()
		: RTT::types::StructTypeInfo< WinchControlDataType >( "WinchControlData" )
	{}
};

/// WinchControl typekit plugin
class WinchControlTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __WINCH_CONTROL_TYPEKIT__
