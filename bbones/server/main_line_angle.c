#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "time_highwind.h"

#include "udp_communication.h"
#include "uart_communication2.h" // has declaration of an error handler function pointer
#include "data_decoding.h"
#include "log.h" // also has some error handling stuff

#define MAX_INPUT_STREAM_SIZE 255
#define UDP_SOCKET_TIMEOUT 1000000000


#ifndef DEBUG 
#define DEBUG 0
#endif

		
/************************************
 * PROTOTYPES
 * **********************************/

static void print_mem(void const *vp, int n);
void write_error(char *file_name,char *message,int err_code);

 /***********************************
  * GLOBALS
  * *********************************/
 
typedef struct{
		int port_number_server_to_armbone;
		int port_number_armbone_to_server;
		char *armbone_ip;
} Connection;

//function pointer to write errors to log
void (*write_error_ptr)(char *,char *,int);

			
 /***********************************
  * MAIN
  * *********************************/
int main(int argc, char *argv[]){

	Connection connection;
	write_error_ptr = &write_error;  //initialize the function pointer to write error
	
	//parse arguments	
	if(argc == 4){
		//first argument is always name of program or empty string
		connection.armbone_ip=argv[1];
		connection.port_number_server_to_armbone=atoi(argv[2]);	
		connection.port_number_armbone_to_server=atoi(argv[3]);
	}else{
			printf("wrong parameters: armbone ip - send port number - receive port number\n");
			exit(EXIT_FAILURE);		
	}
		
	
	/*-------------------------START OF FIRST THREAD: ARMBONE TO SERVER------------------------*/
	
	static UDP udp_server;
	uint8_t input_stream[30];
	timeval tv_now;

	UDP_err_handler(openUDPServerSocket(&udp_server,connection.port_number_armbone_to_server,UDP_SOCKET_TIMEOUT),write_error_ptr);
	
	//init the data decode pointers
	init_decoding();
	
	int err;
	int recv_len;
	size_t data_len = sizeof(input_stream);

	while (1){
		// err = receiveUDPServerData(&udp_server,(void *)&input_stream, sizeof(input_stream)); //blocking !!!
		//1. retreive UDP data form armbone from ethernet port.
		err = receiveUDPServerData(&udp_server,(void *)&input_stream, data_len, &recv_len); //blocking !!!

		if (recv_len != data_len) {
			printf("Wrong number of bytes in received UDP packet!\n");
                	printf("Expected 30 bytes, Received %d bytes!\n", recv_len);
                	err = UDP_ERR_RECV;
		}

		UDP_err_handler(err,write_error_ptr); 
	
		if(err == UDP_ERR_NONE){
			gettimeofday(&tv_now,NULL); //timestamp from receiving to calculate latency
			
			#if DEBUG > 0
			
				printf("message raw: ");
				int i;
				for(i=0;i<input_stream[1];i++){
						printf("%d ",input_stream[i]);
				}
				printf("\n");
				
				printf("start hex: %x\n", input_stream[0]);
				printf("length: %d\n", input_stream[1]);
				printf("send id: %d\n", input_stream[2]);
				printf("message id: %d\n", input_stream[3]);
				printf("checksum1: %d\n", input_stream[input_stream[1]-2]);
				printf("checksum2: %d\n", input_stream[input_stream[1]-1]);
				// printf("%d", input_stream[3]);
				printf("\n");
			
			#endif
			
			//2. decode data 		
			err  = data_decode(input_stream);
			DEC_err_handler(err,write_error_ptr);
	
			if(err==DEC_ERR_NONE){ 
			
				switch_read_write(); //only switch read write if data decoding was succesfull
				Data* data = get_read_pointer();

				if(input_stream[3]==LINE_ANGLE_ID){
					// Send a character (to gpio of arduino)
					// to stop the arduino-timer
					/*
					FILE *myFile;
					myFile = fopen("/dev/ttyUSB0", "w");
					fputs ("a", myFile);
					fclose (myFile);
					*/
					

					printf("LINE_ANGLE_ID content:");
					print_mem((void *)&data->bone_arm.line_angle,sizeof(LINE_ANGLE));
					
					printf("Azimuth %i\n",data->bone_arm.line_angle.azimuth_raw);
					printf("Elevation %i\n",data->bone_arm.line_angle.elevation_raw);
					//char temp[64];
					//timestamp_to_timeString16(data->bone_arm.line_angle.tv,temp);
					//printf("send time: %s\n",temp);
					printf("\n");
				}
									
			}else{
					printf("UNKNOW PACKAGE with id %d\n",input_stream[3]);
					exit(1);
			}
		}
		
	}
	
	UDP_err_handler(closeUDPServerSocket(&udp_server),write_error_ptr);

	/*------------------------END OF FIRST THREAD------------------------*/
	return 0;
}

void write_error(char *file_name,char *message,int err_code)
{
	//TODO: make it thread safe!!
    error_write(file_name,message);
}

static void print_mem(void const *vp, int n)
{
    unsigned char const *p = vp;
	int i;
	
    for (i=0; i<n; i++)
        printf("%d ", p[i]);
    printf("\n");
};




