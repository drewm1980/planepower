#ifndef __DYNAMIC_MHE_TYPEKIT__
#define __DYNAMIC_MHE_TYPEKIT__

#include "DynamicMheDataTypes.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, DynamicMheStateEstimate& l, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("ts_trigger", l.ts_trigger);
			a & make_nvp("ts_entry", l.ts_entry);
			a & make_nvp("ts_elapsed", l.ts_elapsed);

			a & make_nvp("ready", l.ready);
			a & make_nvp("x_hat", l.x_hat);
		}

		template< class Archive >
		void serialize(Archive& a, DynamicMheHorizon& l, unsigned int)
		{
			using boost::serialization::make_nvp;
			
			a & make_nvp("ts_trigger", l.ts_trigger);
			a & make_nvp("ts_entry", l.ts_entry);
			a & make_nvp("ts_elapsed", l.ts_elapsed);

			a & make_nvp("exec_fdb", l.exec_fdb);
			a & make_nvp("exec_prep", l.exec_prep);

			a & make_nvp("ready", l.ready);

			a & make_nvp("solver_status", l.solver_status);
			a & make_nvp("kkt_value", l.kkt_value);
			a & make_nvp("obj_value", l.obj_value);
			a & make_nvp("n_asc", l.n_asc);

			a & make_nvp("x", l.x);
			a & make_nvp("u", l.u);
			a & make_nvp("z", l.z);
			a & make_nvp("y", l.y);
			a & make_nvp("yN", l.yN);
//			a & make_nvp("S", l.S);
//			a & make_nvp("SN", l.SN);

			a & make_nvp("num_imu_samples", l.num_imu_samples);
			a & make_nvp("num_enc_samples", l.num_enc_samples);
			a & make_nvp("num_cam_samples", l.num_cam_samples);
			a & make_nvp("num_las_samples", l.num_las_samples);
			a & make_nvp("num_winch_samples", l.num_winch_samples);

			a & make_nvp("imu_first", l.imu_first);
			a & make_nvp("imu_avg", l.imu_avg);
			a & make_nvp("enc_data", l.enc_data);
			a & make_nvp("cam_markers", l.cam_markers);
			a & make_nvp("cam_pose", l.cam_pose);
			a & make_nvp("las_data", l.las_data);
			a & make_nvp("winch_data", l.winch_data);
			a & make_nvp("controls_avg", l.controls_avg);

			a & make_nvp("dbg_cam_delay", l.dbg_cam_delay);
			a & make_nvp("dbg_imu_delay", l.dbg_imu_delay);
			a & make_nvp("dbg_enc_delay", l.dbg_enc_delay);
			a & make_nvp("dbg_winch_delay", l.dbg_winch_delay);
		}
	}
}

struct DynamicMheStateEstimateInfo
	: public RTT::types::StructTypeInfo< DynamicMheStateEstimate >
{
	DynamicMheStateEstimateInfo()
		: RTT::types::StructTypeInfo< DynamicMheStateEstimate >( "DynamicMheStateEstimate" )
	{}
};

struct DynamicMheHorizonInfo
	: public RTT::types::StructTypeInfo< DynamicMheHorizon >
{
	DynamicMheHorizonInfo()
		: RTT::types::StructTypeInfo< DynamicMheHorizon >( "DynamicMheHorizon" )
	{}
};

/// DynamicMhe typekit plugin
class DynamicMheTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __DYNAMIC_MHE_TYPEKIT__
