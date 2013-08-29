#ifndef __DYNAMIC_MHE_DATA_TYPES__
#define __DYNAMIC_MHE_DATA_TYPES__

#include <vector>

/// This, compact, data structure is intended to be sent to a controller
typedef struct
{
	/// Time-stamps: trigger [ticks], entry [ticks], elapsed [sec];
	double ts_trigger, ts_entry, ts_elapsed;
	/// Ready indicator
	bool ready;
	/// Current state estimate
	std::vector< double > x_hat;
} DynamicMheStateEstimate;

/// A huge data structure for debugging of estimator
typedef struct
{
	/// Time-stamps: trigger [ticks], entry [ticks], elapsed [sec];
	double ts_trigger, ts_entry, ts_elapsed;
	/// Exec times for the MHE: feedback and preparation phase [sec]
	double exec_fdb, exec_prep;
	/// Ready indicator
	bool ready;
	/// Performance indicators
	double kkt_value, obj_value, n_asc;

	/// Horizons of optimization data
	std::vector< double > x; 
	std::vector< double > u; 
	std::vector< double > z; 
	std::vector< double > y;
	std::vector< double > yN;
	std::vector< double > S;
	std::vector< double > SN;

	//
	// Sensor data (ATM, same as in the KinematicMhe component)
	//

	/// Number of data samples
	int num_imu_samples, num_enc_samples, num_cam_samples, num_las_samples;
	/// First sample and averaged IMU samples
	std::vector< double > imu_first, imu_avg;
	/// Encoder measurement
	std::vector< double > enc_data;
	/// Marker positions
	std::vector< double > cam_markers;
	/// Pose and orientation w.r.t anchorframe point
	std::vector< double > cam_pose;
	/// Horizontal and vertical angle from LAS [V]; TODO Convert to [rad]
	std::vector< double > las_data;
} DynamicMheHorizon;

#endif // __DYNAMIC_MHE_DATA_TYPES__
