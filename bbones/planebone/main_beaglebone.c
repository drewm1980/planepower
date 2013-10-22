/*
 * AUTHOR: Jonas Van Pelt
 */

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "udp_communication.h"
#include "uart_communication.h"
#include "log.h"
#include "circular_buffer.h"
#include "data_decoding.h"

#ifndef LOGGING
#define LOGGING 1
#endif

#define CBSIZE 1024 * 16
#define OUTPUT_BUFFER 36
#define MAX_STREAM_SIZE 255
#define UDP_SOCKET_TIMEOUT 1000000000


/************************************
 * PROTOTYPES
 * **********************************/
static void *lisa_to_pc(void *connection);
static void *data_logging_lisa(void *);
static void *data_logging_groundstation(void *arg);
static void enable_ptp();
static void sendError(DEC_errCode err,library lib);
static void switch_cb_lisa_pointers();
static void switch_cb_ground_pointers();
static void write_uart_error(char *file_name,char *message,int err_code);
static void write_udp_error(char *file_name,char *message,int err_code);
static void write_decode_error(char *file_name,char *message,int err_code);
static void write_log_error(char *file_name,char *message,int err_code);

 /***********************************
  * GLOBALS
  * *********************************/

static char FILENAME[] = "main_bealgebone.c";

typedef struct{
		int port_number_lisa_to_pc;
		int port_number_pc_to_lisa;
		char *server_ip;
} Connection;

static Connection connection;


#if LOGGING > 0
//log ping pong buffers for data from lisa
static CircularBuffer cb_data_lisa_ping;
static CircularBuffer cb_data_lisa_pong;
static CircularBuffer *cb_read_lisa = &cb_data_lisa_ping;
static CircularBuffer *cb_write_lisa = &cb_data_lisa_pong;
static int reading_flag_lisa=0;

//log ping pong buffers for data from groundstation
static CircularBuffer cb_data_ground_ping;
static CircularBuffer cb_data_ground_pong;
static CircularBuffer *cb_read_ground = &cb_data_ground_ping;
static CircularBuffer *cb_write_ground = &cb_data_ground_pong;
static int reading_flag_ground=0;
#endif

//function pointer to write errors to log
void (*write_uart_error_ptr)(char *,char *,int);
void (*write_udp_error_ptr)(char *,char *,int);
void (*write_decode_error_ptr)(char *,char *,int);
void (*write_log_error_ptr)(char *,char *,int);

 /***********************************
  * MAIN
  * *********************************/
  

int main(int argc, char *argv[]){
	write_uart_error_ptr = &write_uart_error;  //initialize the function pointer to write error
	write_udp_error_ptr = &write_udp_error; 
	write_decode_error_ptr = &write_decode_error;  
	write_log_error_ptr = &write_log_error;  
 
	//parse arguments
	if(argc == 4){
		//first argument is always name of program or empty string
		connection.server_ip=argv[1];
		connection.port_number_lisa_to_pc=atoi(argv[2]);
		connection.port_number_pc_to_lisa=atoi(argv[3]);
	}else{
			printf("wrong parameters: server ip - send port number - receive port number\n");
			exit(EXIT_FAILURE);
	}

	//init log (mount sd card if necessary)

	int err = init_log();
	LOG_err_handler(err,write_log_error_ptr);

	if(err != LOG_ERR_NONE){
		exit(EXIT_FAILURE);		//mounting SD card failed
	}
	
	enable_ptp();

	#if LOGGING > 0
	//init circular data log buffers
	 cbInit(cb_read_lisa, CBSIZE);
	 cbInit(cb_write_lisa, CBSIZE);
	 cbInit(cb_read_ground, CBSIZE);
	 cbInit(cb_write_ground, CBSIZE);
	 #endif

	err = serial_port_setup();
	UART_err_handler(err,write_uart_error_ptr);
	if(err != UART_ERR_NONE){
		exit(EXIT_FAILURE);
	}

	//thread variables
	pthread_t thread_lisa_to_pc,thread_data_logging_lisa,thread_data_logging_ground;

	//create a second thread which executes lisa_to_pc
	if(pthread_create(&thread_lisa_to_pc, NULL, lisa_to_pc,&connection)) {
		error_write(FILENAME,"error creating lisa thread");
		exit(EXIT_FAILURE);
	}

	#if LOGGING > 0

	//create a third thread which executes data_logging_lisa
	if(pthread_create(&thread_data_logging_lisa, NULL, data_logging_lisa,NULL)) {
		error_write(FILENAME,"error creating lisa logging thread");
		exit(EXIT_FAILURE);
	}

	//create a fourth thread which executes data_logging_groundstation
	if(pthread_create(&thread_data_logging_ground, NULL, data_logging_groundstation,NULL)) {
		error_write(FILENAME,"error creating groundstation logging thread");
		exit(EXIT_FAILURE);
	}

	#endif
	/*-------------------------START OF FIRST THREAD: PC TO LISA------------------------*/
	static UDP udp_server;
	uint8_t input_stream[OUTPUT_BUFFER];

	ElemType cb_elem = {0};

	//init the data decode pointers
	init_decoding();

	UDP_err_handler(openUDPServerSocket(&udp_server,connection.port_number_pc_to_lisa,UDP_SOCKET_TIMEOUT),write_udp_error_ptr);

	while(1){

		//1. retreive UDP data form PC from ethernet port.
		err=receiveUDPServerData(&udp_server,(void *)&input_stream,sizeof(input_stream)); //blocking !!!
		UDP_err_handler(err,write_udp_error_ptr);

		if(err==UDP_ERR_NONE){

			#if LOGGING > 0
			
			if(!cbIsFull(cb_write_ground)){
				 memcpy (&cb_elem.value, &input_stream, sizeof(input_stream));
				 cbWrite(cb_write_ground, &cb_elem);
			 }else{
				if(reading_flag_ground==0){
					switch_cb_ground_pointers();
				}else{
					printf("GROUND WRITE WAS NOT READY \n");
				}
			 }
			
			#endif

			int new_length = strip_timestamp(input_stream); //lisa expects a package without a timestamp

			UART_err_handler(serial_port_write(input_stream,new_length),write_uart_error_ptr);
		}

	}
	UART_err_handler(serial_port_close(),write_uart_error_ptr);
	UDP_err_handler(closeUDPServerSocket(&udp_server),write_udp_error_ptr);
	/*------------------------END OF FIRST THREAD------------------------*/


	//wait for the second thread to finish
	if(pthread_join(thread_lisa_to_pc, NULL)) {
		error_write(FILENAME,"error joining thread_lisa_to_pc");
	}

	#if LOGGING > 0

	//wait for the third thread to finish
	if(pthread_join(thread_data_logging_lisa, NULL)) {
		error_write(FILENAME,"error joining thread_data_logging_lisa");
	}


	//wait for the fourth thread to finish
	if(pthread_join(thread_data_logging_ground, NULL)) {
		error_write(FILENAME,"error joining thread_data_logging_ground");
	}

	//free circular buffers
	cbFree(cb_read_lisa);
	cbFree(cb_write_lisa);
	cbFree(cb_read_ground);
	cbFree(cb_write_ground);

	#endif

	return 0;
}

/************************************
 * FUNCTIONS
 * **********************************/
static void *lisa_to_pc(void *arg){
/*-------------------------START OF SECOND THREAD: LISA TO PC------------------------*/

	static UDP udp_client;
	int message_length;
	ElemType cb_elem = {0};
	uint8_t input_buffer[INPUT_BUFFER_SIZE];

	UDP_err_handler(openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_lisa_to_pc,UDP_SOCKET_TIMEOUT),write_udp_error_ptr);

	while(1)
	{
		message_length = serial_input_get_lisa_data(input_buffer); //blocking !!!
		if(message_length > 0){
			
			//to test latency from lisa pull pin high when airspeed package arrives
			/*if(serial_input.buffer[3]==AIRSPEED_ETS){
				if (test==0){
					system("echo 1 > /sys/class/gpio/gpio60/value");
					test=1;
				}else{
					system("echo 0 > /sys/class/gpio/gpio60/value");
					test=0;
				}
			}*/

			//add timestamp
			message_length=add_timestamp(input_buffer);

			//send data to eth port using UDP
			UDP_err_handler(sendUDPClientData(&udp_client,input_buffer,message_length),write_udp_error_ptr);

			#if LOGGING > 0

			//write the data to circual buffer for log thread
			 if(!cbIsFull(cb_write_lisa)){
				 memcpy (&cb_elem.value, input_buffer, message_length);
				 cbWrite(cb_write_lisa, &cb_elem);
			 }else{
				if(reading_flag_lisa==0){
					switch_cb_lisa_pointers();
					//printf("switching lisa pointers\n");
				}else{
					printf("LISA WRITE WAS NOT READY \n");
					exit(1); //FOR DEBUGGING
				}
			 }

			#endif
		}else{
		//send error message to server: not receiving data on uart port
			printf("error on uart, see log...\n"); //FOR DEBUGGING
			UART_err_handler(message_length,write_uart_error_ptr);
		}

	}

	UART_err_handler(serial_port_close(),write_uart_error_ptr);

	UDP_err_handler(closeUDPClientSocket(&udp_client),write_udp_error_ptr);

	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/

}

#if LOGGING > 0

static void *data_logging_lisa(void *arg){
/*-------------------------START OF THIRD THREAD: LISA TO PC LOGGING------------------------*/

	ElemType cb_elem = {0};
	LOG_err_handler(open_data_lisa_log(),write_log_error_ptr);

	while(1){
		if (!cbIsEmpty(cb_read_lisa)) {
			reading_flag_lisa=1;
			cbRead(cb_read_lisa, &cb_elem);
			LOG_err_handler(write_data_lisa_log(cb_elem.value,cb_elem.value[1]),write_log_error_ptr);
			usleep(100);
		}else{
			reading_flag_lisa=0;
			usleep(1000);
		}	
	}
	LOG_err_handler(close_data_lisa_log(),write_log_error_ptr);

	return NULL;
/*-------------------------END OF THIRD THREAD: LISA TO PC LOGGING------------------------*/
}

static void *data_logging_groundstation(void *arg){
/*-------------------------START OF FOURTH THREAD: GROUNDSTATION TO LISA LOGGING------------------------*/

	ElemType cb_elem = {0};
	LOG_err_handler(open_data_groundstation_log(),write_log_error_ptr);
	
	while(1){
		if (!cbIsEmpty(cb_read_ground)) {
			reading_flag_ground=1;
			cbRead(cb_read_ground, &cb_elem);
			LOG_err_handler(write_data_groundstation_log(cb_elem.value,cb_elem.value[1]),write_log_error_ptr);
			usleep(100);
		}else{
			reading_flag_ground=0;
			usleep(1000);
		}	
	}

	LOG_err_handler(close_data_groundstation_log(),write_log_error_ptr);

	return NULL;
/*-------------------------END OF FOURTH THREAD: GROUNDSTATION TO LISA LOGGING------------------------*/

}

#endif


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
		openUDPClientSocket(&udp_client,connection.server_ip,connection.port_number_lisa_to_pc,UDP_SOCKET_TIMEOUT);
		sendUDPClientData(&udp_client,&encoded_data,message_length);
		closeUDPClientSocket(&udp_client);
}

static void enable_ptp(){
	int err = system("./ptpd-2.2.0/src/ptpd2 -b eth0 -g -y 0 -D -f /var/log/ptpd.log -L");
	if(err!=0 && err != 768){ //768 = ptp already running
		printf("could not enable ptp: error code %d\n",err);
		exit(EXIT_FAILURE);
	}
}

static void switch_cb_lisa_pointers(){
		CircularBuffer *temp = cb_read_lisa;
		cb_read_lisa = cb_write_lisa;
		cb_write_lisa = temp;
}

static void switch_cb_ground_pointers(){
		CircularBuffer *temp = cb_read_ground;
		cb_read_ground = cb_write_ground;
		cb_write_ground = temp;
}

static void write_uart_error(char *file_name,char *message,int err_code)
{
	//TODO: make it thread safe!!
    error_write(file_name,message);
    sendError(err_code,UART_L);
}

static void write_udp_error(char *file_name,char *message,int err_code)
{
	//TODO: make it thread safe!!
    error_write(file_name,message);
    //udp errors cannot be send
}

static void write_decode_error(char *file_name,char *message,int err_code)
{
	//TODO: make it thread safe!!
    error_write(file_name,message);
    sendError(err_code,DECODE_L);
}

static void write_log_error(char *file_name,char *message,int err_code)
{
	//TODO: make it thread safe!!
    error_write(file_name,message);
    sendError(err_code,LOG_L);
}
