#ifndef __DYNAMIC_MPC_TYPEKIT__
#define __DYNAMIC_MPC_TYPEKIT__

#include "DynamicMpcDataTypes.hpp"
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/TypekitPlugin.hpp>

namespace boost
{
	namespace serialization
	{
		template< class Archive >
		void serialize(Archive& a, DynamicMpcHorizon& l, unsigned int)
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

			a & make_nvp("x_hat", l.x_hat);

			a & make_nvp("x", l.x);
			a & make_nvp("u", l.u);
			a & make_nvp("z", l.z);
			a & make_nvp("y", l.y);
			a & make_nvp("yN", l.yN);
//			a & make_nvp("S", l.S);
//			a & make_nvp("SN", l.SN);
		}
	}
}

struct DynamicMpcHorizonInfo
	: public RTT::types::StructTypeInfo< DynamicMpcHorizon >
{
	DynamicMpcHorizonInfo()
		: RTT::types::StructTypeInfo< DynamicMpcHorizon >( "DynamicMpcHorizon" )
	{}
};

/// DynamicMpc typekit plugin
class DynamicMpcTypekit
	: public RTT::types::TypekitPlugin
{
public:
	virtual std::string getName();

	virtual bool loadTypes();
	virtual bool loadConstructors();
	virtual bool loadOperators();
};

#endif // __DYNAMIC_MPC_TYPEKIT__
