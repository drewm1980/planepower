#ifndef __KINEMATIC_MHE_DATA_TYPE__
#define __KINEMATIC_MHE_DATA_TYPE__

#include <vector>

typedef struct
{
	/// Number of data samples
	int num_imu_samples, num_enc_samples, num_cam_samples;
	/// First sample and averaged IMU samples
	std::vector< double > imu_first, imu_avg;
	/// Encoder measurement
	std::vector< double > enc_data;
	/// Marker positions
	std::vector< double > cam_markers;
	/// Pose and orientation w.r.t anchorframe point
	std::vector< double > cam_pose;
	/// Time-stamps: trigger [ticks], entry [ticks], elapsed[sec];
	double ts_trigger, ts_entry, ts_elapsed;
} KinematicMheDataType;

#endif // __KINEMATIC_MHE_DATA_TYPE__
