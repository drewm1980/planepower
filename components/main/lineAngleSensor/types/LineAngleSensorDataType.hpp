#ifndef __LINEANGLESENSORDATATYPE__
#define __LINEANGLESENSORDATATYPE__

#include <vector>

/// Custom data type for the encoder component
typedef struct
{
	/// Measured angles [rad]
	float angle_hor, angle_ver;
	/// Time-stamps: trigger [ticks] and elapsed [sec]
	double ts_trigger, ts_elapsed;
} LineAngleSensorDataType;

/// Averaging function
inline void average(std::vector< LineAngleSensorDataType >& vec, LineAngleSensorDataType& avg, unsigned size)
{
	unsigned sz = size < vec.size() ? size : vec.size();

	avg.angle_hor = avg.angle_ver = 0.0;

	for (unsigned i = 0; i < sz; ++i)
	{
		avg.angle_hor += vec[ i ].angle_hor;
		avg.angle_ver += vec[ i ].angle_ver;
	}
	avg.angle_hor /= (float)sz;
	avg.angle_ver /= (float)sz;

	avg.ts_trigger = vec[sz - 1].ts_trigger;
	avg.ts_elapsed = vec[sz - 1].ts_elapsed;
}

#endif // __LINEANGLESENSORDATATYPE__
