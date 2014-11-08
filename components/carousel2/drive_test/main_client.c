#include <stdio.h>
#include <stdint.h>

#include "udp_communication.h"
#include <stdlib.h> //exit(0);

static UDP udp_client;
 
void quit(){
	fprintf(stderr,"memory exhausted\n");
	exit(1);
}

int getSpeedFromUser(){
	int max = 20;
	char* speedInput = (char*)malloc(max);
	if(speedInput==0) quit();
	int i=0;
	int len = 0;
	printf("Please enter the speed as a percentage of the nominal speed\n");
	while(1){
		int c = getchar();
		if(isspace(c) || c == EOF){
			speedInput[i]=0;
			len=i;
			break;
		}
		speedInput[i]=c;
		if(i==max-1){
			max = max+max;
			speedInput = (char*)realloc(speedInput,max);
			if(speedInput==0){quit();}
		}
		i++;
	}
	
	int speedpercent = atoi(speedInput);
	int nominalSpeed = 1073741824;
	int n = speedpercent/100.0*nominalSpeed;
	return n;
}

int main(int argc, char *argv[]){

	int port_number = 2000;
	char *ip_address = "192.168.0.1";
	printf("Opening client\n");
	openUDPClientSocket(&udp_client,ip_address,port_number);
	int i;
	for(i=0; i<10;i++){
		uint32_t n1 = (uint32_t) getSpeedFromUser();
		uint32_t n2 = 0;// (uint32_t) getSpeedFromUser();
		unsigned char bytes[8];
		bytes[0] = (n1 >> 24) & 0xFF;
		bytes[1] = (n1 >> 16) & 0xFF;
		bytes[2] = (n1 >> 8) & 0xFF;
		bytes[3] = n1 & 0xFF;
		bytes[4] = (n2 >> 24) & 0xFF;
		bytes[5] = (n2 >> 16) & 0xFF;
		bytes[6] = (n2 >> 8) & 0xFF;
		bytes[7] = n2 & 0xFF;
		printf("Sending data %d out of 10\n",i+1);
		if(sendUDPClientData(&udp_client,bytes,sizeof(bytes)))
			printf("sending of data failed\n");
	}
	printf("Closing client\n");
	closeUDPClientSocket(&udp_client);
	
return 0;	
}

