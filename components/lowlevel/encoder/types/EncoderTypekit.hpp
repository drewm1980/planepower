#ifndef __ENCODERTYPEKIT__
#define __ENCODERTYPEKIT__

#include "EncoderDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, EncoderDataType& e, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("theta", e.theta);
			a & make_nvp("sin_theta", e.sin_theta);
			a & make_nvp("cos_theta", e.cos_theta);

			a & make_nvp("omega", e.omega);
			a & make_nvp("omega_filt_rpm", e.omega_filt_rpm);

			a & make_nvp("ts_trigger", e.ts_trigger);
			a & make_nvp("ts_elapsed", e.ts_elapsed);
		}
	}
}

struct EncoderDataTypeInfo
	: public RTT::types::StructTypeInfo< EncoderDataType >
{
	EncoderDataTypeInfo()
		: RTT::types::StructTypeInfo< EncoderDataType >( "EncoderData" )
	{}
};

/// Encoder typekit plugin
class EncoderTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __ENCODERTYPEKIT__
