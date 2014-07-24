#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <math.h>

#include "armbone_lisa_communication.hpp"

#define DEBUG 0


using namespace std;

ArmboneLisaReceiver::ArmboneLisaReceiver()
{
	port_number = 7777;
	openUDPServerSocket(&udp_server, port_number, UDP_SOCKET_TIMEOUT);
	init_decoding(); 
	cout << "Opened server socket on ground station for reading from the Armbone's Lisa..." << endl;

}

ArmboneLisaReceiver::~ArmboneLisaReceiver()
{
	closeUDPServerSocket(&udp_server);
}

int ArmboneLisaReceiver::read(ImuGyro *imu_gyro, ImuMag *imu_mag, ImuAccel *imu_accel)	
{
	int message_type = -1;
	int err = UDP_ERR_NONE;
	int recv_len;
	size_t data_len = sizeof(input_stream);
	receiveUDPServerData(&udp_server,(void *)&input_stream, data_len, &recv_len); //blocking !!!

	if (recv_len != data_len) {
        	printf("Wrong number of bytes in received UDP packet!\n");
                printf("Expected %lu bytes, Received %d bytes!\n", data_len, recv_len);
                err = UDP_ERR_RECV;
        }

	if ( err != UDP_ERR_NONE)
        {
                exception();
        }
	//2. decode data                
        err  = data_decode(input_stream);
        Data* data = get_read_pointer();

	
        if(err==DEC_ERR_NONE){
	        switch_read_write(); //only switch read write if data decoding was succesfull
		if(input_stream[3]==IMU_GYRO_RAW)
		{	

			
			imu_gyro->gp = convertRawGyro(data->lisa_plane.imu_gyro_raw.gp);
			imu_gyro->gq = -1.0 * convertRawGyro(data->lisa_plane.imu_gyro_raw.gq);
			imu_gyro->gr = -1.0 * convertRawGyro(data->lisa_plane.imu_gyro_raw.gr);
			message_type = 1;
#if DEBUG > 0	
			printf("IMU_GYRO_RAW gp: %i\n",data->lisa_plane.imu_gyro_raw.gp);
			printf("IMU_GYRO_RAW gq: %i\n",data->lisa_plane.imu_gyro_raw.gq);
			printf("IMU_GYRO_RAW gr: %i\n",data->lisa_plane.imu_gyro_raw.gr);
#endif

		} 
		else if (input_stream[3] == IMU_MAG_RAW)
		{
			// Add 183 (Offset of the x component)
			imu_mag->mx = convertRawMagX(data->lisa_plane.imu_mag_raw.mx + 183);
			// Add 100 (Offset of the y component)
			imu_mag->my = convertRawMagY(data->lisa_plane.imu_mag_raw.my + 100);
			// Subtract 55 (Offset of the z component)
			imu_mag->mz = convertRawMagZ(data->lisa_plane.imu_mag_raw.mz - 55);
			// Function to get the current angle of the arm
			// Offsets are eliminated in the function
			imu_mag->angle = convertRawMagToAngle(data->lisa_plane.imu_mag_raw.mx, data->lisa_plane.imu_mag_raw.my);
			message_type = 2;
#if DEBUG > 0	
			printf("IMU_MAG_RAW mx: %i\n",data->lisa_plane.imu_mag_raw.mx);
			printf("IMU_MAG_RAW my: %i\n",data->lisa_plane.imu_mag_raw.my);
			printf("IMU_MAG_RAW mz: %i\n",data->lisa_plane.imu_mag_raw.mz);
#endif
		}
		else if (input_stream[3] == IMU_ACCEL_RAW)
		{
			// Add 45 (Offset of the x component)
			imu_accel->ax = convertRawAccel(data->lisa_plane.imu_accel_raw.ax + 45);
			// Subtract 25 (Offset of the y component)
			imu_accel->ay = convertRawAccel(data->lisa_plane.imu_accel_raw.ay - 25);
			// Add 364 (Offset of the z component)
			imu_accel->az = -1.0 * convertRawAccel(data->lisa_plane.imu_accel_raw.az + 364);
			message_type = 3;
#if DEBUG > 0	
			printf("IMU_ACCEL_RAW ax: %i\n",data->lisa_plane.imu_accel_raw.ax);
			printf("IMU_ACCEL_RAW ay: %i\n",data->lisa_plane.imu_accel_raw.ay);
			printf("IMU_ACCEL_RAW az: %i\n",data->lisa_plane.imu_accel_raw.az);
#endif
		} 
		else 
		{
 			printf("UNKNOW PACKAGE with id %d\n",input_stream[3]);
                }
	} else {
		printf("Decoding Error (Code %i)\n", err);
	}

	return message_type;

}


double ArmboneLisaReceiver::convertRawGyro(int raw_data) 
{
	double data = ((double) raw_data/pow(2,15)) * 2000.0 * (PI/180.0) + 0.0202; //	Rad/s
	return data;
}

double ArmboneLisaReceiver::convertRawAccel(int raw_data) 
{
	double data = (((double)raw_data)/1010.0) * -4.795; //	m/s²
	return data;
}


// Different computingfunctions for the Magnetometer components because they have not the same range
// Maybe because there are disturbing obstacles on or nearby the carousel??

double ArmboneLisaReceiver::convertRawMagX(int raw_data) 
{
	double data = (((double)raw_data)/243.0) * 20.0; //	uT
	return data;
}

double ArmboneLisaReceiver::convertRawMagY(int raw_data) 
{
	double data = (((double)raw_data)/211.0) * 20.0; //	uT
	return data;
}

double ArmboneLisaReceiver::convertRawMagZ(int raw_data) 
{
	// Add 48 because thats the vertical component of the magnetic field
	// Don't know the factor of raw_data (0.1 for now). 
	// Could be measuered by climbing up the carousel and rotating the LISA around that axis
	double data = ((double)raw_data) * 0.1 + 48.0; //	uT
	return data;
}

double ArmboneLisaReceiver::convertRawMagToAngle(int raw_X, int raw_Y) 
{
	// Eliminate offsets & normalize values
	double mapped_X = ((double) raw_X + 182.5)/242.5;
	double mapped_Y = ((double) raw_Y + 99.5)/210.5;
	// To make sure that the acos function only gets values between -1 and 1
	mapped_X = fmax(-0.99999, mapped_X);
	mapped_X = fmin(0.99999, mapped_X);
	double angle;
	// Compute angle
	if (mapped_Y >= 0) 
	{
		//angle = -(mapped_X-1.0) * 90.0;
		angle = acos(mapped_X) * 180/PI;
	}
	if (mapped_Y < 0) 
	{
		//angle = ((mapped_X + 1.0) * 90.0) + 180;
		angle = acos(-mapped_X) * 180/PI + 180;
	}
	return angle;
}
