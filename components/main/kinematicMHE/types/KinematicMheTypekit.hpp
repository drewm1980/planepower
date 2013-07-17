#ifndef __KINEMATIC_MHE_TYPEKIT__
#define __KINEMATIC_MHE_TYPEKIT__

#include "KinematicMheDataType.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, KinematicMheDataType& l, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("ts_trigger", l.ts_trigger);
			a & make_nvp("ts_entry", l.ts_entry);
			a & make_nvp("ts_elapsed", l.ts_elapsed);

			a & make_nvp("num_imu_samples", l.num_imu_samples);
			a & make_nvp("num_enc_samples", l.num_enc_samples);
			a & make_nvp("num_cam_samples", l.num_cam_samples);

			a & make_nvp("imu_first", l.imu_first);
			a & make_nvp("imu_avg", l.imu_avg);
			a & make_nvp("enc_data", l.enc_data);
			a & make_nvp("cam_markers", l.cam_markers);
			a & make_nvp("cam_pose", l.cam_pose);
		}
	}
}

struct KinematicMheDataTypeInfo
	: public RTT::types::StructTypeInfo< KinematicMheDataType >
{
	KinematicMheDataTypeInfo()
		: RTT::types::StructTypeInfo< KinematicMheDataType >( "KinematicMheData" )
	{}
};

/// KinematicMhe typekit plugin
class KinematicMheTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __KINEMATIC_MHE_TYPEKIT__
