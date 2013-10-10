#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

#include "siemens_communication.hpp"

using namespace std;

// Compute checksum for our message type.
// Note, the message must contain only uint32_t for this to work!!!
// Returns true if existing checksum was already correct
// This should be called ~before byteswapping when sending
//                   and ~after byteswapping when recieving
template <class Packet>
bool recompute_checksum(Packet *c)
{
	int numentries = sizeof(Packet)/sizeof(uint32_t)-1;
	uint32_t sum = 0;
	for(int i=0; i<numentries; i++)
	{
		sum += *((uint32_t *) c + i);
	}
	if(c->checksum == sum)
	{
		return true;
	} else {
		c->checksum = sum;
		return false;
	}
}

// bswap32 all of the entries of a stuct containing only (u)int32_t's
// Not, the message must contain only uint32_t for this to work!!!
template <class Packet>
void bswap_packet(Packet *c)
{
	int numentries = sizeof(Packet)/sizeof(uint32_t);
#if CHECKSUM
	numentries -=1;
#endif
	uint32_t* p_c = (uint32_t*) c;
	for(int i=0; i<numentries; i++)
	{
		 p_c[i] = __builtin_bswap32(p_c[i]);
	}
}

Siemens::Siemens()
{ 
	strncpy(ip_address,"192.168.000.001",sizeof(ip_address)-1);
	ip_address[15] = '\0';
	port_number = 2000;
	openUDPClientSocket(&udp_client,ip_address,port_number);
	openUDPServerSocket(&udp_server,port_number);
	currentWinchCommand = 0;
	currentCarouselCommand = 0;
}
Siemens::~Siemens()
{
	send_reference_speeds(0,0);
	closeUDPClientSocket(&udp_client);
	closeUDPServerSocket(&udp_server);
}

int Siemens::send_reference_speeds(double winchSpeed, double carouselSpeed)
{
	UDPSendPacket udpsc;
	memset(&udpsc,0,sizeof(udpsc));
	int32_t n1 = winchSpeed/nominalWinchSpeed*nominalCommand;
	int32_t n2 = carouselSpeed/nominalCarouselSpeed*nominalCommand;
	n2 = 0; // TEMPORARY SAFETY HACK
	udpsc.winchSpeedReference = n1;
	udpsc.carouselSpeedReference = n2;
#if CHECKSUM
	recompute_checksum(&udpsc);
#endif
	bswap_packet(&udpsc);

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

// Blocks until a UDP packet arrives!
void Siemens::read(SiemensDriveState* ds)
{
	UDPReceivePacket c;
	memset(ds,0,sizeof(SiemensDriveState));
	receiveUDPServerData(&udp_server,(void *)&c,sizeof(c)); //blocking !!!
	bswap_packet(&c);
#if CHECKSUM
	bool success = recompute_checksum(&c);	
	if(!success) printf("Warning, a UDP packet failed the checksum!!!\n");
#endif
	
	ds->winchSpeedSmoothed = ((double) c.winchSpeedSmoothed)/(nominalCommand);
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

