#ifndef __MCUHANDLERTYPEKIT__
#define __MCUHANDLERTYPEKIT__

#include "McuHandlerDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, ControlSurfacesValues& l, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("ua1", l.ua1);
			a & make_nvp("ua2", l.ua2);
			a & make_nvp("ue", l.ue);

			a & make_nvp("d_ua1", l.d_ua1);
			a & make_nvp("d_ua2", l.d_ua2);
			a & make_nvp("d_ue", l.d_ue);
		}

		template< class Archive >
		void serialize(Archive& a, McuHandlerDataType& mht, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("gyro_x", mht.gyro_x);
			a & make_nvp("gyro_y", mht.gyro_y);
			a & make_nvp("gyro_z", mht.gyro_z);
			
			a & make_nvp("accl_x", mht.accl_x);
			a & make_nvp("accl_y", mht.accl_y);
			a & make_nvp("accl_z", mht.accl_z);
			
			a & make_nvp("ctrl", mht.ctrl);
			
			a & make_nvp("ts_trigger", mht.ts_trigger);
			a & make_nvp("ts_elapsed", mht.ts_elapsed);
		}
	}
}

struct McuHandlerDataTypeInfo
	: public RTT::types::StructTypeInfo< McuHandlerDataType >
{
	McuHandlerDataTypeInfo()
		: RTT::types::StructTypeInfo< McuHandlerDataType >( "McuHandlerData" )
	{}
};

struct ControlSurfacesValuesTypeInfo
	: public RTT::types::StructTypeInfo< ControlSurfacesValues >
{
    ControlSurfacesValuesTypeInfo()
		: RTT::types::StructTypeInfo< ControlSurfacesValues >( "ControlSurfacesValues" )
	{}
};

/// McuHandler typekit plugin
class McuHandlerTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __MCUHANDLERTYPEKIT__
