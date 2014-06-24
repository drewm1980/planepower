#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

#include "siemens_communication.hpp"

using namespace std;

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

SiemensSender::SiemensSender()
{ 
	strncpy(ip_address,SIEMENS_DRIVES_SEND_IP_ADDRESS,sizeof(ip_address)-1);
	ip_address[15] = '\0';
	port_number = 2000;
	unsigned int timeout = 1000000; // us
	openUDPClientSocket(&udp_client,ip_address,port_number,timeout);
	currentWinchCommand = 0;
	currentCarouselCommand = 0;

	// Note, we do NOT want to send a zero command at startup,
	// since then we could not use this to manually gradually stop a drive that was left running
	// by something else (i.e. another program that crashed.
}
SiemensSender::~SiemensSender()
{
	stop_drives();
	closeUDPClientSocket(&udp_client);
}

void SiemensSender::stop_drives()
{
	send_reference_speeds(0,0);
}

int SiemensSender::send_reference_speeds(double winchSpeed, double carouselSpeed)
{
	UDPSendPacket udpsc;
	memset(&udpsc,0,sizeof(udpsc));

	// Convert nominal output
	
	// Clamp the values to nominal ShaftSpeeds , to make fucking sure there is no datatype wraparound.
	if(winchSpeed>100) winchSpeed = 100;
	if(winchSpeed<-100) winchSpeed = -100;
	if(carouselSpeed>100) carouselSpeed = 100;
	if(carouselSpeed<-100) carouselSpeed = -100;

	// Convert percentages to command values
	int32_t n1 = winchSpeed/100.0/nominalWinchSpeed*nominalCommand;
	int32_t n2 = carouselSpeed/100.0/nominalCarouselSpeed*nominalCommand;

	// Send the udp packet
	udpsc.winchSpeedReference = n1;
	udpsc.carouselSpeedReference = n2;
	bswap_packet(&udpsc);
	if(sendUDPClientData(&udp_client, &udpsc, sizeof(udpsc)))
	{
		cout << "Sending of data failed!" << endl;
		return -1;
	} else {
		currentWinchCommand = winchSpeed; // (signed) percentage of nominal
		currentCarouselCommand = carouselSpeed; // (signed) percentage of nominal
		return 0;
	}
}
int SiemensSender::send_winch_reference_speed(double winch_speed)
{
	return send_reference_speeds(winch_speed, currentCarouselCommand);
}
int SiemensSender::send_carousel_reference_speed(double carousel_speed)
{
	return send_reference_speeds(currentWinchCommand, carousel_speed);
}

// Note:  THINK VERY HARD before modifying these.
// This implementation delibrately:
//		* avoids errors when UDP message fails to even get sent
//		* avoids unchanged values drifting when you call these repeatedly
int SiemensSender::send_calibrated_speeds(double winch_speed, double carousel_speed)
{
	return send_reference_speeds(winch_speed*100.0/nominalWinchSpeed,
							carousel_speed*100.0/nominalCarouselSpeed);
}
int SiemensSender::send_winch_calibrated_speed(double winch_speed)
{
	return send_reference_speeds(winch_speed*100.0/nominalWinchSpeed, currentCarouselCommand);
}
int SiemensSender::send_carousel_calibrated_speed(double carousel_speed)
{
	return send_reference_speeds(currentWinchCommand, carousel_speed*100.0/nominalCarouselSpeed);
}

int SiemensSender::write(SiemensDriveCommand command)
{
	return send_calibrated_speeds(command.winchSpeedReference, 
						   command.carouselSpeedReference);
}

SiemensReceiver::SiemensReceiver()
{ 
	port_number = 2000;
	unsigned int timeout = 1000000; // us
	openUDPServerSocket(&udp_server,port_number,timeout);
	cout << "Opened server socket on ground station for reading from the PLC..." << endl;
}
SiemensReceiver::~SiemensReceiver()
{
	closeUDPServerSocket(&udp_server);
}

// Blocks until a UDP packet arrives!
void SiemensReceiver::read(SiemensDriveState* ds)
{
	UDPReceivePacket c;
	memset(ds,0,sizeof(SiemensDriveState));
	receiveUDPServerData(&udp_server,(void *)&c,sizeof(c)); //blocking !!!
	/*cout << "CSSP " << hex << uppercase << c.carouselSpeedSetpoint << endl;
	cout << "CSS "  << c.carouselSpeedSmoothed << endl;
	cout << "CEP " << c.carouselEncoderPosition << endl;
	cout << "CT " << c.carouselTorque << endl;
	cout << "WSSP " << c.winchSpeedSetpoint << endl;
	cout << "WSS " << c.winchSpeedSmoothed << endl;
	cout << "WEP " << c.winchEncoderPosition << endl;
 	cout << "WT " << c.winchTorque << endl;*/
	bswap_packet(&c);
	/*cout << "bswap ################ " << endl;
	cout << "CSSP " << c.carouselSpeedSetpoint << endl;
	cout << "CSS "  << c.carouselSpeedSmoothed << endl;
	cout << "CEP " << c.carouselEncoderPosition << endl;
	cout << "CT " << c.carouselTorque << endl;
	cout << "WSSP " << c.winchSpeedSetpoint << endl;
	cout << "WSS " << c.winchSpeedSmoothed << endl;
	cout << "WEP " << c.winchEncoderPosition << endl;
 	cout << "WT " << c.winchTorque << endl;*/

	// NOTE: UNITS OF THESE ARE TO BE DETERMINED FROM STARTER! Evrerything beside the Positions has to be proofen
	ds->winchSpeedSetpoint = ((double) c.winchSpeedSetpoint)/nominalCommand * rpm2mps;//in m/s
	ds->winchSpeedSmoothed = ((double) c.winchSpeedSmoothed)/nominalCommand * rpm2mps;//in m/s
	ds->winchEncoderPosition = ((double) c.winchEncoderPosition) * 20 * PI / (246610 - 875);//in rad (experimatal data)
	ds->winchTorque = ((double) c.winchTorque)/nominalCommand / winchGearRatio;//Torque on winch drum in Nm
	ds->winchCurrent = ((double) c.winchCurrent)/nominalCommand;//in Ampere


	ds->carouselSpeedSetpoint = ((double) c.carouselSpeedSetpoint) /nominalCommand* rpm2radps;//in rad/s
	ds->carouselSpeedSmoothed = ((double) c.carouselSpeedSmoothed)/nominalCommand/(carouselGearRatio);//(Pseudo-)arm speed in ad/s
	ds->carouselEncoderPosition = ((double) c.carouselEncoderPosition) * 20 * PI / (626074111 - 164054527);//(Pseudo-)arm Position in rad (experimental data)
	ds->carouselTorque = ((double) c.carouselTorque)/nominalCommand/carouselGearRatio;//Torque on arm shaft in Nm
	ds->carouselCurrent = ((double) c.carouselCurrent)/nominalCommand;//in Ampere

}

void SiemensReceiver::handle_32bit_rollover(EncoderState *e, uint32_t smallCounts)
{
	const int64_t max_change = 0x3FFFFFFF;
	int64_t diff = smallCounts - e->smallCountsLast;
	if(diff > max_change) e->bigCounts -= 0x0000FFFF;
	if(diff < -max_change) e->bigCounts += 0x0000FFFF;
	e->bigCounts += smallCounts;
	e->smallCountsLast = smallCounts;
}

