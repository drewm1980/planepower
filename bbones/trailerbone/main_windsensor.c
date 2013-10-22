#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udp_communication.h"
#include "uart_communication.h"
#include "log.h"
#include "circular_buffer.h"
#include "data_decoding.h"

#define MAX_STREAM_SIZE 255
#define UDP_SOCKET_TIMEOUT 1000000000

static void sendError(DEC_errCode err,library lib);
static void write_uart_error(char *file_name,char *message,int err_code);
static void write_udp_error(char *file_name,char *message,int err_code);
static void write_decode_error(char *file_name,char *message,int err_code);

static char FILENAME[] = "main_windsensor.c";

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
   
int main(int argc, char *argv[]){
	write_uart_error_ptr = &write_uart_error;  //initialize the function pointer to write error
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

	/*-------------------------WINDSENSOR TO SERVER------------------------*/
	static UDP udp_client;
	int message_length;
	uint8_t input_buffer[INPUT_BUFFER_SIZE];
	uint8_t encoded_data[INPUT_BUFFER_SIZE];
	UART_err_handler(serial_port_setup(),write_uart_error_ptr);

	UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_send,UDP_SOCKET_TIMEOUT),write_udp_error_ptr);

	while(1)
	{
		message_length = serial_input_get_windsensor_data(input_buffer); //blocking !!!
		if(message_length > 0){
			
			//translate data structure from windsensor to structure
			//transform structure to format that we can send to server, starbyte, sender id, length, timestamp, ....
			
			NMEA_asci_encode(input_buffer,encoded_data);
			int i;
			printf("output before sending\n");
			for(i=0;i<encoded_data[LENGTH_INDEX];i++){
					printf(" %d",encoded_data[i]);
			}printf("\n");
			
			//send data to eth port using UDP
			UDP_err_handler(sendUDPClientData(&udp_client,encoded_data,encoded_data[LENGTH_INDEX]),write_udp_error_ptr);
		
		}else{
			//send error message to server: not receiving data on uart port
			printf("error on uart, see log...\n"); //FOR DEBUGGING
			UART_err_handler(message_length,write_uart_error_ptr);
		}

	}

	UART_err_handler(serial_port_close(),write_uart_error_ptr);
	UDP_err_handler(closeUDPClientSocket(&udp_client),0);

	return 0;
}

/************************************
 * FUNCTIONS
 * **********************************/

static void sendError(DEC_errCode err,library lib){

		static UDP udp_client;
		int message_length;
		uint8_t encoded_data[MAX_STREAM_SIZE];
		Data data;
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
    error_write(file_name,message);
    //udp errors cannot be send
}

static void write_decode_error(char *file_name,char *message,int err_code)
{
    error_write(file_name,message);
    sendError(err_code,DECODE_L);
}



