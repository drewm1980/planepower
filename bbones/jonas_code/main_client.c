#include <stdio.h>

#include "udp_communication.h"
#include "communication_datatypes.h"
#include<stdlib.h> //exit(0);


static UDP udp_client;
 

int main(int argc, char *argv[]){
	
	int port_number;
	char *ip_address;
	
	if(argc == 3){
		//first argument is always name of program or empty string
		ip_address=argv[1];
		port_number=atoi(argv[2]);		
	}else{
			printf("wrong parameters: enter destination ip adress and port number\n");
			exit(1);
	}
	
	union Serial_output {
		char buffer[30];
		uint32_t set_servo_buffer[7];
	} serial_output;
	
	openUDPClientSocket(&udp_client,ip_address,port_number);
	
	
	while(1){
		usleep(500);
		serial_output.set_servo_buffer[0]=100;
		serial_output.set_servo_buffer[1]=0;
		serial_output.set_servo_buffer[2]=0;
		serial_output.set_servo_buffer[3]=0;
		serial_output.set_servo_buffer[4]=0;
		serial_output.set_servo_buffer[5]=0;
		serial_output.set_servo_buffer[6]=0;
		serial_output.buffer[28]=0;
		serial_output.buffer[29]=0;
		sendUDPClientData(&udp_client,&serial_output,sizeof(serial_output));
	}
	
	closeUDPClientSocket(&udp_client);
	
return 0;	
}

