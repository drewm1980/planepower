#include <stdio.h>
#include <stdlib.h>
#include "udp_communication.hpp"
#include <sys/time.h>
#include <time.h>

static UDP udp_server;

// This is a wire format defined in the PLC and starter software
struct UDPReceive{
	int32_t winchSpeedSmoothed;
	uint32_t winchEncoderPosition;
	int32_t carouselSpeedSmoothed;
	uint32_t carouselEncoderPosition;
//	uint32_t checksum;
};

// Compute checksum for our message type.
// Not, the message must contain only uint32_t for this to work!!!
//void recompute_checksum(UDPSpeedCommand *c)
//{
//	int numentries = sizeof(UDPSpeedCommand)/sizeof(uint32_t)-1;
//	uint32_t sum = 0;
//	for(int i=0; i<numentries; i++)
//	{
//		sum += *((uint32_t *) c + i);
//		c->checksum = sum;
//	}
//}


int main(int argc, char *argv[])
{
	int port_number=2000;
	UDPReceive c;

	openUDPServerSocket(&udp_server,port_number);

	timespec t1, t2;
	clock_gettime(CLOCK_REALTIME, &t1); // Works on Linux
	for(int i=0; i<1000; i++)
	{
		receiveUDPServerData(&udp_server,(void *)&c,sizeof(c)); //blocking !!!
	}
	clock_gettime(CLOCK_REALTIME, &t2); // Works on Linux
	time_t dsec = t2.tv_sec-t1.tv_sec;
	long dnsec = t2.tv_nsec-t1.tv_nsec;
	
	double dt;
	//if(dnsec>0) dt = dsec + dnsec*1e-9



	while(1){
		printf("\nWaiting for data...\n");
		fflush(stdout);

		receiveUDPServerData(&udp_server,(void *)&c,sizeof(c)); //blocking !!!
		c.winchSpeedSmoothed = __builtin_bswap32(c.winchSpeedSmoothed);
		c.winchEncoderPosition = __builtin_bswap32(c.winchEncoderPosition);
		c.carouselSpeedSmoothed = __builtin_bswap32(c.carouselSpeedSmoothed);
		c.carouselEncoderPosition = __builtin_bswap32(c.carouselEncoderPosition);
		//double winchSpeed = ((double) c.winchSpeedSmoothed)/(0x40000000);
		//printf("winchSpeed: %f\n",winchSpeed);
		printf("winchPosition: %u\n",c.winchEncoderPosition);
	}

	closeUDPServerSocket(&udp_server);
}

