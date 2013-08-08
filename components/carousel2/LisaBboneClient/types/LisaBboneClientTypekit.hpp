#ifndef __LISA_BBONE_CLIENT_TYPEKIT__
#define __LISA_BBONE_CLIENT_TYPEKIT__

#include "LisaBboneClientDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, LisaBboneClientDataType& mht, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("gyro_x", mht.gyro_x);
			a & make_nvp("gyro_y", mht.gyro_y);
			a & make_nvp("gyro_z", mht.gyro_z);
			
			a & make_nvp("accl_x", mht.accl_x);
			a & make_nvp("accl_y", mht.accl_y);
			a & make_nvp("accl_z", mht.accl_z);
						
			a & make_nvp("ts_trigger", mht.ts_trigger);
			a & make_nvp("ts_elapsed", mht.ts_elapsed);
		}
	}
}

struct LisaBboneClientDataTypeInfo
	: public RTT::types::StructTypeInfo< LisaBboneClientDataType >
{
	LisaBboneClientDataTypeInfo()
		: RTT::types::StructTypeInfo< LisaBboneClientDataType >( "LisaBboneClientData" )
	{}
};

/// LisaBboneClient typekit plugin
class LisaBboneClientTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __LISA_BBONE_CLIENT_TYPEKIT__
