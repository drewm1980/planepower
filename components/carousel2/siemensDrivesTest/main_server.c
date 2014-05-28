#include <stdio.h>
#include<stdlib.h> //exit(0);

#include "udp_communication.h"

static UDP udp_server;
 

int main(int argc, char *argv[]){
	
	int port_number=2000;

	//create test struct to send test data
	/*Barometer barometer;
	Lisa_message lisa_message;*/
	
	/*uint32_t test;*/
	uint32_t receive[8];
	unsigned int timeout = 1000000;
	openUDPServerSocket(&udp_server,port_number,timeout);

	while(1){
	 	printf("\nWaiting for data...\n");
		fflush(stdout);

		receiveUDPServerData(&udp_server,(void *)&receive,sizeof(receive)); //blocking !!!
		
		//print details of the client/peer and the data received
		printf("start: %x ", receive[0]);
		
		
	}
	
	closeUDPServerSocket(&udp_server);
	
	
return 0;	
}

