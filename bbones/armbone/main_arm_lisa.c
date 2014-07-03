#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "udp_communication.h"
#include "log.h"
#include "circular_buffer.h"
#include "data_decoding.h"
#include "uart_communication.h"

#define MAX_STREAM_SIZE 255
#define LINE_ANGLE_BUFFER_SIZE 8		
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

//function pointer to write errors to log
void (*write_uart_error_ptr)(char *,char *,int);
void (*write_udp_error_ptr)(char *,char *,int);
void (*write_decode_error_ptr)(char *,char *,int);

/***********************************
 * MAIN
 * *********************************/

int main(int argc, char *argv[])
{
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
	static UDP udp_client;
	//int message_length;
	int err;
	int message_length;
	uint8_t input_buffer[INPUT_BUFFER_SIZE];				
	uint8_t encoded_data[MAX_STREAM_SIZE];

	UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_send,UDP_SOCKET_TIMEOUT),write_udp_error_ptr);
	// initialize the serial port
	init_decoding();
	err = serial_port_setup();
	UART_err_handler(err, write_uart_error_ptr);
	if(err != UART_ERR_NONE) {
		exit(EXIT_FAILURE);
	}
	
	printf("Entering real-time while loop.  Will not do any printing in the loop. \n");
	while(1)
	{	
		message_length = serial_input_get_lisa_data(input_buffer);
		if(message_length > 0){
            		// printf("Received serial input with message id: %c\n", input_buffer[3]);
	    		//Decode messages to see what we receive
			int message_id = input_buffer[3];
            		int err_decode = data_decode(input_buffer);
            		if (err_decode != DEC_ERR_NONE)
            		{
                		printf("Error while decoding data messages.");
            		}



			//printf("%.2X  %.2X  %.2X  %.2X  \n",input_buffer[3],input_buffer[2],input_buffer[1],input_buffer[0]); //////////checking

			data_encode(input_buffer,INPUT_BUFFER_SIZE,encoded_data,LISA,message_id);
	

			// printf("%.2X  %.2X  %.2X  %.2X  \n",encoded_data[7],encoded_data[6],encoded_data[5],encoded_data[4]); //////////checking

#if 0
			printf("output before sending\n");
			for(int i=0;i<encoded_data[LENGTH_INDEX];i++){
				printf(" %d",encoded_data[i]);
			}printf("\n");
#endif

			//send data to eth port using UDP
			if (message_id == IMU_GYRO_RAW || message_id == IMU_MAG_RAW || message_id == IMU_ACCEL_RAW) 
			{
				UDP_err_handler(sendUDPClientData(&udp_client,encoded_data,encoded_data[LENGTH_INDEX]),write_udp_error_ptr);
			}
		}else{
			UART_err_handler(err,write_uart_error_ptr);
		}

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


