#ifndef __MCHUHANDLERDATATYPE__
#define __MCHUHANDLERDATATYPE__

/// Input/output data type for control surfaces
struct ControlSurfacesValues
{
	ControlSurfacesValues()
		: ua1(0.0), ua2(0.0), ue(0.0), d_ua1(0.0), d_ua2(0.0), d_ue(0.0),
		  der_ctrl(false)
	{}

	void reset()
	{
		ua1 = ua2 = ue = d_ua1 = d_ua2 = d_ue = 0.0;
		der_ctrl = false;
	}

	/// Control surfaces' values [rad]
	float ua1, ua2, ue;
	/// Control surcafes derivative values [rad/s]
	float d_ua1, d_ua2, d_ue;
	/// Indircator for derivative control, when only derivatives are used
	bool der_ctrl;
};

/// Custom data type for the MCU handler
typedef struct
{
	/// Gyro readings in [rad/s]
	float gyro_x, gyro_y, gyro_z;
	/// Acceleration readings in [m/s^2]
	float accl_x, accl_y, accl_z;
	/// Applied controls
	ControlSurfacesValues ctrl;
	/// Time-stamps: trigger [ticks] and elapsed time [s]
	double ts_trigger, ts_elapsed;
} McuHandlerDataType;

#endif // __MCHUHANDLERDATATYPE__
