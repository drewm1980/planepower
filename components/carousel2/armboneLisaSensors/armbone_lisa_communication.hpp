#ifndef ARMBONE_LISA_COMMUNICATION_HPP
#define ARMBONE_LISA_COMMUNICATION_HPP

#include <stdint.h>

#include "ImuGyro.h"
#include "ImuMag.h"
#include "ImuAccel.h"

#include "udp_communication.h"
#include "data_decoding.h"

#define MAX_INPUT_STREAM_SIZE 255
#define UDP_SOCKET_TIMEOUT 1000000000
#define PI 3.1415

class ArmboneLisaReceiver
{
	public:
		ArmboneLisaReceiver();	
		~ArmboneLisaReceiver();
	
		// returns either:
		// -1 (error)
		// 1 (if new gyro value exists)
		// 2 (if new mag value exists)
		// 3 (if new accel value exists)
		int read(ImuGyro *imu_gyro, ImuMag *imu_mag, ImuAccel *imu_accel);	
		double convertRawGyro(int raw_data);
		double convertRawAccel(int raw_data);
		double convertRawMagToAngle(int raw_X, int raw_Y);

	protected:

	private:
		UDP udp_server;
		int port_number;
		uint8_t input_stream[18];
};

#endif
