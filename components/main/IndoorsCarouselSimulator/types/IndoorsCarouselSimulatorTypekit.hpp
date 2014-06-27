#ifndef __ICS_TYPEKIT__
#define __ICS_TYPEKIT__

#include "IndoorsCarouselSimulatorDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/CArrayTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>
#include <rtt/types/BoostArrayTypeInfo.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, IndoorsCarouselSimulatorDataType& l, unsigned int)
		{
			using boost::serialization::make_nvp;
						
			a & make_nvp("ts_trigger", l.ts_trigger);
			a & make_nvp("ts_elapsed", l.ts_elapsed);
		}
	}
}

struct IndoorsCarouselSimulatorDataTypeInfo
	: public RTT::types::StructTypeInfo< IndoorsCarouselSimulatorDataType >
{
	IndoorsCarouselSimulatorDataTypeInfo()
		: RTT::types::StructTypeInfo< IndoorsCarouselSimulatorDataType >( "IndoorsCarouselSimulatorData" )
	{}
};

/// IndoorsCarouselSimulator typekit plugin
class IndoorsCarouselSimulatorTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __ICS_TYPEKIT__
