#ifndef __DYNAMIC_MPC_DATA_TYPES__
#define __DYNAMIC_MPC_DATA_TYPES__

#include <vector>

/// A huge data structure for debugging of controller
typedef struct
{
	/// Time-stamps: trigger [ticks], entry [ticks], elapsed [sec];
	double ts_trigger, ts_entry, ts_elapsed;
	/// Exec times for the MPC: feedback and preparation phase [sec]
	double exec_fdb, exec_prep;
	/// Ready indicator
	bool ready;
	/// Error code
	int solver_status;
	/// Performance indicators
	double kkt_value, obj_value, n_asc;

	/// Current state estimate
	std::vector< double > x_hat;

	/// Horizons of optimization data
	std::vector< double > x; 
	std::vector< double > u; 
	std::vector< double > z; 
	std::vector< double > y;
	std::vector< double > yN;
	std::vector< double > S;
	std::vector< double > SN;
} DynamicMpcHorizon;

#endif // __DYNAMIC_MPC_DATA_TYPES__
