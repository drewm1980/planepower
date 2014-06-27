#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

#include "armbone_communication.hpp"

using namespace std;

ArmboneReceiver::ArmboneReceiver()
{
	port_number = 8888;
	openUDPServerSocket(&udp_server, port_number, UDP_SOCKET_TIMEOUT);
	init_decoding(); 
	cout << "Opened server socket on ground station for reading from the Armbone..." << endl;

}

ArmboneReceiver::~ArmboneReceiver()
{
	closeUDPServerSocket(&udp_server);
}

void ArmboneReceiver::read(LineAngles * lineangles) 
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

        // UDP_err_handler(err,write_error_ptr);
	if ( err != UDP_ERR_NONE)
        {
                exception();
        }


	//2. decode data                
        err  = data_decode(input_stream);
        // DEC_err_handler(err,write_error_ptr);
        Data* data = get_read_pointer();
	
        if(err==DEC_ERR_NONE){
	        switch_read_write(); //only switch read write if data decoding was succesfull
		if(input_stream[3]==LINE_ANGLE_ID)
		{	
			/*
			printf("LINE_ANGLE_ID content:\n");
                        // print_mem((void *)&data->bone_arm.line_angle,sizeof(LINE_ANGLE));
                        printf("Azimuth %i\n",data->bone_arm.line_angle.azimuth_raw);
                        printf("Elevation %i\n",data->bone_arm.line_angle.elevation_raw);
                        printf("\n");
			*/
			
			// Convert the rawvalues to radians

			
			lineangles->azimuth = (double) raw_to_radians(data->bone_arm.line_angle.azimuth_raw);
			lineangles->elevation = (double) raw_to_radians(data->bone_arm.line_angle.elevation_raw);
		} else {
 			printf("UNKNOW PACKAGE with id %d\n",input_stream[3]);
                       }
	}

}

float ArmboneReceiver::raw_to_radians(uint16_t raw) 
{
	const unsigned int counts = 0x1 << 16;
	const float rescale = 2.0*3.1415 / counts;
	return ((float)raw) * rescale;
}
