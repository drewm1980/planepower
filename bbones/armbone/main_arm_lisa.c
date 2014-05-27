#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "udp_communication.h"
#include "log.h"
#include "circular_buffer.h"
#include "data_decoding.h"
#include "uart_communication.h"

#define MAX_STREAM_SIZE 255
#define LISA_DATA_SIZE 18
#define UDP_SOCKET_TIMEOUT 1000000000

/************************************
 * PROTOTYPES
 * **********************************/

static void sendError(DEC_errCode err,library lib);
static void write_uart_error(char *file_name,char *message,int err_code);
static void write_udp_error(char *file_name,char *message,int err_code);
// Commented because tup complains:
// Trying to get it to work and later clean up
//
// static void write_decode_error(char *file_name,char *message,int err_code);

/***********************************
 * GLOBALS
 * *********************************/

// static char FILENAME[] = "main_arm.c";

typedef struct{
	int port_number_send;
	char *server_ip;
}Connection;

static Connection connection;
static UDP udp_client;

//function pointer to write errors to log
void (*write_uart_error_ptr)(char *,char *,int);
void (*write_udp_error_ptr)(char *,char *,int);
void (*write_decode_error_ptr)(char *,char *,int);

/***********************************
 * MAIN
 * *********************************/

void clean_exit(int signum)
{	
	printf("Exiting program...\n");
 	printf("%i\n", signum);
	closeUDPClientSocket(&udp_client);
	serial_port_close();
	usleep(5000);
	printf("done!\n");
	exit(0);
}


int main(int argc, char *argv[])
{	
	signal(SIGINT, clean_exit);
	
	write_uart_error_ptr = &write_uart_error; //initialize the function pointer to write error
	write_udp_error_ptr = &write_udp_error;

	//parse arguments
	if(argc == 3){
		//first argument is always name of program or empty string
		connection.server_ip=argv[1];
		connection.port_number_send=atoi(argv[2]);
	}else{
		printf("wrong parameters: server ip - send port number - receive port number\n");
		exit(EXIT_FAILURE);
	}

	/*-------------------------LINE-ANGLE SENSOR TO SERVER------------------------*/
	// int err;
	int message_length;
	int message_id = 0;
	uint8_t input_buffer[LISA_DATA_SIZE];				

	UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_send,UDP_SOCKET_TIMEOUT),write_udp_error_ptr);
	// initialize the serial port
	init_decoding();
	UART_err_handler(serial_port_setup(), write_uart_error_ptr);
	
	printf("Entering real-time while loop.  Will not do any printing in the loop. \n");
	while(1)
	{	
		message_length = serial_input_get_lisa_data(input_buffer); 
		if(message_length == 18){
			message_id = input_buffer[3];
			if ((message_id >= 203) && (message_id <= 205)) {
#if DEBUG > 0
            		printf("Received serial input with message id: %i\t", input_buffer[3]);
            		printf("length: %i\n", message_length);
#endif
// We don't decode the data on the armbone
// instead we send directly to the groundstation
#if 0
				
	    			//Decode messages to see what we receive
            			err_decode = data_decode(input_buffer);
            			if (err_decode != DEC_ERR_NONE)
            			{
                			printf("Error while decoding data messages.");
            			}



			//printf("%.2X  %.2X  %.2X  %.2X  \n",input_buffer[3],input_buffer[2],input_buffer[1],input_buffer[0]); //////////checking

				//data_encode(input_buffer,INPUT_BUFFER_SIZE,encoded_data,LISA,message_id);
				data_encode(input_buffer,LISA_DATA_SIZE,encoded_data,LISA,message_id);
				printf("Encoded data size: %i\n", sizeof(encoded_data));

			// printf("%.2X  %.2X  %.2X  %.2X  \n",encoded_data[7],encoded_data[6],encoded_data[5],encoded_data[4]); //////////checking

			
			printf("output before sending\n");
			for(int i=0;i<encoded_data[LENGTH_INDEX];i++){
				printf(" %d",encoded_data[i]);
			}printf("\n");
			
			Data* data = get_read_pointer();
			switch_read_write();

			if(message_id==IMU_GYRO_RAW)
                {
			/*
                        imu_gyro->gp_raw = (double) data->lisa_plane.imu_gyro_raw.gp;
                        imu_gyro->gq_raw = (double) data->lisa_plane.imu_gyro_raw.gq;
                        imu_gyro->gr_raw = (double) data->lisa_plane.imu_gyro_raw.gr;
                        message_type = 1;
			*/
#if DEBUG > 0   
                        printf("IMU_GYRO_RAW gp: %i\n",data->lisa_plane.imu_gyro_raw.gp);
                        printf("IMU_GYRO_RAW gq: %i\n",data->lisa_plane.imu_gyro_raw.gq);
                        printf("IMU_GYRO_RAW gr: %i\n",data->lisa_plane.imu_gyro_raw.gr);
#endif

                }
                else if (message_id == IMU_MAG_RAW)
                {	
			/*
                        imu_mag->mx_raw = (double) data->lisa_plane.imu_mag_raw.mx;
                        imu_mag->my_raw = (double) data->lisa_plane.imu_mag_raw.my;
                        imu_mag->mz_raw = (double) data->lisa_plane.imu_mag_raw.mz;
                        message_type = 2;
			*/
#if DEBUG > 0   
                        printf("IMU_MAG_RAW mx: %i\n",data->lisa_plane.imu_mag_raw.mx);
                        printf("IMU_MAG_RAW my: %i\n",data->lisa_plane.imu_mag_raw.my);
                        printf("IMU_MAG_RAW mz: %i\n",data->lisa_plane.imu_mag_raw.mz);
#endif
                }
                else if (message_id == IMU_ACCEL_RAW)
                {	
			/*
                        imu_accel->ax_raw = (double) data->lisa_plane.imu_accel_raw.ax;
                        imu_accel->ay_raw = (double) data->lisa_plane.imu_accel_raw.ay;
                        imu_accel->az_raw = (double) data->lisa_plane.imu_accel_raw.az;
                        message_type = 3;
			*/
#if DEBUG > 0   
                        printf("IMU_ACCEL_RAW ax: %i\n",data->lisa_plane.imu_accel_raw.ax);
                        printf("IMU_ACCEL_RAW ay: %i\n",data->lisa_plane.imu_accel_raw.ay);
                        printf("IMU_ACCEL_RAW az: %i\n",data->lisa_plane.imu_accel_raw.az);
#endif
                }
                else
                {
                        printf("UNKNOW PACKAGE with id %d\n",message_id);
                }

			
			
#endif

			//send data to eth port using UDP
				if (message_id == IMU_GYRO_RAW || message_id == IMU_MAG_RAW || message_id == IMU_ACCEL_RAW) 
				{
					UDP_err_handler(sendUDPClientData(&udp_client,input_buffer,18),write_udp_error_ptr);
				}else{
					//UART_err_handler(err,write_uart_error_ptr);
					//printf("Error in message_id: %i\n", message_id);
					//return 1;
				}
			}	
		} else {
			UART_err_handler(message_length, write_uart_error_ptr);
			//printf("Error in message_length: %i\n", message_length);
			//printf("ID: %i\n", message_id);
            		//printf("Error in decoding: %i\n", err_decode);
			//return 1;
		}
		usleep(2000);
	}
	UDP_err_handler(closeUDPClientSocket(&udp_client),0);
	usleep(5000);
	return 0;
}

/************************************
 * FUNCTIONS
 * **********************************/

static void sendError(DEC_errCode err,library lib)
{
	static UDP udp_client;
	int message_length;
	uint8_t encoded_data[MAX_STREAM_SIZE];
	Beagle_error error_message;

	//encode an error package
	error_message.library=lib;
	error_message.error_code=err;
	data_encode((uint8_t *)&error_message,sizeof(error_message),encoded_data,2,2);
	message_length=sizeof(encoded_data);

	//send errorcode to server, no error handling here otherwise we get infinite loop try to send error
	openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_send,UDP_SOCKET_TIMEOUT);
	sendUDPClientData(&udp_client,&encoded_data,message_length);
	closeUDPClientSocket(&udp_client);
}

static void write_uart_error(char *file_name,char *message,int err_code)
{
	error_write(file_name,message);
	sendError(err_code,UART_L);
}

static void write_udp_error(char *file_name,char *message,int err_code)
{
        err_code = 0;
	error_write(file_name,message);
}


/*
static void write_decode_error(char *file_name,char *message,int err_code)
{
	error_write(file_name,message);
	sendError(err_code,DECODE_L);
}
*/


