#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

#include "siemens_communication.hpp"

// The following values must be configured to match in the siemens "Starter" app.
#define nominalWinchSpeed 0.0 // Nominal drive shaft rotation speed
#define nominalCarouselSpeed 0.0 // Nominal drive shaft rotation speed
#define nominalCommand 0x40000000 // This is the command corresponding to the nominal drive shaft rotation speed

using namespace std;

Siemens::Siemens()
{ 
	strncpy(ip_address,"192.168.000.001",sizeof(ip_address)-1);
	ip_address[15] = '\0';
	port_number = 2000;
	openUDPClientSocket(&udp_client,ip_address,port_number);
	currentWinchCommand = 0;
	currentCarouselCommand = 0;
}
Siemens::~Siemens()
{
	send_reference_speeds(0,0);
	closeUDPClientSocket(&udp_client);
}

int Siemens::send_reference_speeds(double winchSpeed, double carouselSpeed)
{
	UDPSpeedCommand udpsc;
	memset(&udpsc,0,sizeof(udpsc));
	int32_t n1 = winchSpeed/nominalWinchSpeed*nominalCommand;
	//int32_t n2 = carouselSpeed/nominalCarouselSpeed*nominalCommand;
	udpsc.winchSpeedReference = __builtin_bswap32(n1);
	//udpsc.carouselSpeedReference = __builtin_bswap32(n2);
	udpsc.carouselSpeedReference = 0;
//		recompute_checksum(&udpsc);

	if(sendUDPClientData(&udp_client, &udpsc, sizeof(udpsc)))
	{
		cout << "Sending of data failed!" << endl;
		return -1;
	} else {
		currentWinchCommand = winchSpeed;
		currentCarouselCommand = carouselSpeed;
		return 0;
	}
}

int Siemens::send_winch_reference_speed(double winch_speed)
{
	return send_reference_speeds(winch_speed, currentCarouselCommand);
}

int Siemens::send_carousel_reference_speed(double carousel_speed)
{
	return send_reference_speeds(currentWinchCommand, carousel_speed);
}

void Siemens::read_positions(double *tether_length, double *cos_delta, double *sin_delta)
{

}

void Siemens::handle_32bit_rollover(EncoderState *e, uint32_t smallCounts)
{
	const int64_t max_change = 0x3FFFFFFF;
	int64_t diff = smallCounts - e->smallCountsLast;
	if(diff > max_change) e->bigCounts -= 0x0000FFFF;
	if(diff < -max_change) e->bigCounts += 0x0000FFFF;
	e->bigCounts += smallCounts;
	e->smallCountsLast = smallCounts;
}

// Compute checksum for our message type.
// Not, the message must contain only uint32_t for this to work!!!
void Siemens::recompute_checksum(UDPSpeedCommand *c)
{
	int numentries = sizeof(UDPSpeedCommand)/sizeof(uint32_t)-1;
	uint32_t sum = 0;
	for(int i=0; i<numentries; i++)
	{
		sum += *((uint32_t *) c + i);
//		c->checksum = sum;
	}
}

