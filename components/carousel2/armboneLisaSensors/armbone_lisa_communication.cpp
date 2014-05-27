#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

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

void ArmboneLisaReceiver::read(ImuGyro *imu_gyro, ImuMag *imu_mag, ImuAccel *imu_accel)	
{
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

			
			imu_gyro->gp_raw = data->lisa_plane.imu_gyro_raw.gp;
			imu_gyro->gq_raw = data->lisa_plane.imu_gyro_raw.gq;
			imu_gyro->gr_raw = data->lisa_plane.imu_gyro_raw.gr;
#if DEBUG > 0	
			printf("IMU_GYRO_RAW gp: %i\n",data->lisa_plane.imu_gyro_raw.gp);
			printf("IMU_GYRO_RAW gq: %i\n",data->lisa_plane.imu_gyro_raw.gq);
			printf("IMU_GYRO_RAW gr: %i\n",data->lisa_plane.imu_gyro_raw.gr);
#endif

		} 
		else if (input_stream[3] == IMU_MAG_RAW)
		{
			imu_mag->mx_raw = data->lisa_plane.imu_mag_raw.mx;
			imu_mag->my_raw = data->lisa_plane.imu_mag_raw.my;
			imu_mag->mz_raw = data->lisa_plane.imu_mag_raw.mz;
#if DEBUG > 0	
			printf("IMU_MAG_RAW mx: %i\n",data->lisa_plane.imu_mag_raw.mx);
			printf("IMU_MAG_RAW my: %i\n",data->lisa_plane.imu_mag_raw.my);
			printf("IMU_MAG_RAW mz: %i\n",data->lisa_plane.imu_mag_raw.mz);
#endif
		}
		else if (input_stream[3] == IMU_ACCEL_RAW)
		{
			imu_accel->ax_raw = data->lisa_plane.imu_accel_raw.ax;
			imu_accel->ay_raw = data->lisa_plane.imu_accel_raw.ay;
			imu_accel->az_raw = data->lisa_plane.imu_accel_raw.az;
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

}
