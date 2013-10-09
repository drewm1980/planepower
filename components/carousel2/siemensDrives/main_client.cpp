#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "udp_communication.hpp"
#include <stdlib.h> 
#include <ctype.h>

static UDP udp_client;
 
void quit(){
	fprintf(stderr,"memory exhausted\n");
	exit(1);
}

// Returns speed as a signed integer in the format the PLC and drives want,
// up to endianness
int32_t getSpeedFromUser(){
	printf("Please enter the speed as a percentage of the nominal speed:\n");
	float speedpercent;
	scanf("%f",&speedpercent);
	const int nominalSpeed = 0x40000000;
	if (speedpercent>100.0) speedpercent=100.0;
	if (speedpercent<-100.0) speedpercent=-100.0;
	int32_t n = speedpercent/100.0*nominalSpeed;
	return n;
}

// This is a wire format we defined to hold the references for both motors
struct UDPSpeedCommand {
	uint32_t winchSpeedReference;
	uint32_t carouselSpeedReference;
//	uint32_t checksum;
};

// Compute checksum for our message type.
// Not, the message must contain only uint32_t for this to work!!!
void recompute_checksum(UDPSpeedCommand *c)
{
	int numentries = sizeof(UDPSpeedCommand)/sizeof(uint32_t)-1;
	uint32_t sum = 0;
	for(int i=0; i<numentries; i++)
	{
		sum += *((uint32_t *) c + i);
//		c->checksum = sum;
	}
}


int main(int argc, char *argv[])
{
	UDPSpeedCommand c;
	memset(&c,sizeof(c),0);
	int port_number = 2000;
	char ip_address[] = "192.168.0.1";
	printf("Opening client\n");
	openUDPClientSocket(&udp_client,ip_address,port_number);
	bool cont=1;
	while(cont){
		c.winchSpeedReference = __builtin_bswap32(getSpeedFromUser());
		//c.carouselSpeedReference = __builtin_bswap32(getSpeedFromUser());
		c.carouselSpeedReference = __builtin_bswap32(0);
//		recompute_checksum(&c);

		if(sendUDPClientData(&udp_client, &c, sizeof(c)))
			printf("sending of data failed\n");
		printf("Continue?\n");
		char cc = getchar();
//		if(cc!='y' && cc!='Y') cont=0;
	}
	printf("Closing client\n");
	closeUDPClientSocket(&udp_client);
	
	return 0;	
}

