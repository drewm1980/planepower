#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "udp_communication.hpp"
#include <stdlib.h> 
#include <ctype.h>

static UDP udp_client;
 
void quit(){
	fprintf(stderr,"memory exhausted\n");
	exit(1);
}

int32_t getSpeedFromUser(){
	printf("Please enter the speed as a percentage of the nominal speed:\n");
	float speedpercent;
	scanf("%f",&speedpercent);
	const int nominalSpeed = 1073741824;
	if (speedpercent>100.0) speedpercent=100.0;
	if (speedpercent<-100.0) speedpercent=-100.0;
	int32_t n = speedpercent/100.0*nominalSpeed;
	return n;
}

int main(int argc, char *argv[])
{
	int port_number = 2000;
	char ip_address[] = "192.168.0.1";
	printf("Opening client\n");
	openUDPClientSocket(&udp_client,ip_address,port_number);
	bool cont=1;
	while(cont){
		int32_t n = getSpeedFromUser();
		int32_t n_swapped = __builtin_bswap32(n);

		if(sendUDPClientData(&udp_client, &n_swapped, sizeof(n_swapped)))
			printf("sending of data failed\n");
		printf("Continue?\n");
		char c = getchar();
		if(c!='y' || c!='Y') cont=0;
	}
	printf("Closing client\n");
	closeUDPClientSocket(&udp_client);
	
	return 0;	
}

