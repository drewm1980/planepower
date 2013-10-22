#include <stdio.h>
#include <stdlib.h>

#include "udp_communication.h"
//#include "communication_datatypes.h"
#include<stdlib.h> //exit(0);
#include "data_decoding.h"

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
	
	uint8_t data[255]; 
	int i;
	openUDPClientSocket(&udp_client,ip_address,port_number);

	for(i=0;i<255;i++)
	{
		data[i]=i; //creating random data
	}
		
	while(1){
		sleep(1);
		
		sendUDPClientData(&udp_client,&data,sizeof(data));
	}
	
	closeUDPClientSocket(&udp_client);
	
return 0;	
}

