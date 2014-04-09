#ifndef __DYNAMIC_MPC_DATA_TYPES__
#define __DYNAMIC_MPC_DATA_TYPES__

#include <vector>

/// A huge data structure for debugging of controller
typedef struct
{
	/// Time-stamps: trigger [ticks], entry [ticks], elapsed [sec];
	double ts_trigger, ts_entry, ts_elapsed;
	/// Exec times for the MPC: feedback and preparation phase [sec]
	float exec_fdb, exec_prep;
	/// Ready indicator
	bool ready;
	/// Error code
	int solver_status;
	/// Performance indicators
	float kkt_value, obj_value, n_asc;

	/// Current state estimate
	std::vector< float > x_hat;

	/// Horizons of optimization data
	std::vector< float > x; 
	std::vector< float > u; 
	std::vector< float > z; 
	std::vector< float > y;
	std::vector< float > yN;
//	std::vector< float > S;
//	std::vector< float > SN;
} DynamicMpcHorizon;

#endif // __DYNAMIC_MPC_DATA_TYPES__
