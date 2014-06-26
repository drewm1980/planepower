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

int SiemensSender::send_reference_speeds(double winchSpeed, double carouselSpeed)//in percent (-100 to 100)
{
	UDPSendPacket udpsc;
	memset(&udpsc,0,sizeof(udpsc));

	// Convert nominal output
	
	// Clamp the values to nominal ShaftSpeeds , to make fucking sure there is no datatype wraparound.
	if(winchSpeed>100) winchSpeed = 100;
	if(winchSpeed<-100) winchSpeed = -100;
	if(carouselSpeed>100) carouselSpeed = 100;
	if(carouselSpeed<-100) carouselSpeed = -100;

	// Note, there are "Hard Limits" that are implemented on the carousel
	// to limit the values to something sane, but not necesssarily safe.
	// i.e. it will not be a million rpm or something.
	//		These are set in registers p1082[0] of each drive in starter, aka "n_max"
	//		in the Setpoint Channel -> Speed Limit for each drive.

	// Implement "Soft Limits" that can be changed later as needed.
	// Note, at this point in the code these are percentages.
	int winchSoftSpeedLimit = 0; // We are not even using the winch for ball control yet
	int carouselSoftSpeedLimit = 50;
	if(winchSpeed>winchSoftSpeedLimit) winchSpeed = winchSoftSpeedLimit;
	if(winchSpeed<-winchSoftSpeedLimit) winchSpeed = -winchSoftSpeedLimit;
	if(carouselSpeed>carouselSoftSpeedLimit) carouselSpeed = carouselSoftSpeedLimit;
	if(carouselSpeed<-carouselSoftSpeedLimit) carouselSpeed = -carouselSoftSpeedLimit;

	// Convert percentages to command values
	int32_t n1 = winchSpeed/100.0*winchSpeedAt100/nominalWinchSpeed*nominalCommand;
	int32_t n2 = carouselSpeed/100.0*carouselSpeedAt100/nominalCarouselSpeed*nominalCommand;

	// Send the udp packet
	udpsc.winchSpeedSetpoint = n1;
	udpsc.carouselSpeedSetpoint = n2;
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
int SiemensSender::send_calibrated_speeds(double winch_speed, double carousel_speed)// in m/s and rad/s
{
	return send_reference_speeds(winch_speed*100.0/winchSpeedAt100,
							carousel_speed*100.0/carouselSpeedAt100);
}
int SiemensSender::send_winch_calibrated_speed(double winch_speed)
{
	return send_reference_speeds(winch_speed*100.0/winchSpeedAt100, currentCarouselCommand);
}
int SiemensSender::send_carousel_calibrated_speed(double carousel_speed)
{
	return send_reference_speeds(currentWinchCommand, carousel_speed*100.0/carouselSpeedAt100);
}

int SiemensSender::write(SiemensDriveCommand command)
{
	return send_calibrated_speeds(command.winchSpeedSetpoint, 
						   command.carouselSpeedSetpoint);
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
	int recv_len;
	size_t data_len = sizeof(c);
	receiveUDPServerData(&udp_server,(void *)&c, data_len, &recv_len); //blocking !!!


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
	ds->winchSpeedSetpoint = ((double) c.winchSpeedSetpoint) * nominalWinchSpeed / nominalCommand;//in m/s
	ds->winchSpeedSmoothed = ((double) c.winchSpeedSmoothed) * nominalWinchSpeed / nominalCommand;//in m/s
	ds->winchEncoderPosition = ((double) c.winchEncoderPosition) * 20 * PI / (246610 - 875);//in rad (experimatal data)
	ds->winchTorque = ((double) c.winchTorque) * nominalWinchTorque / nominalCommand;//Torque on winch drum in Nm
	ds->winchCurrent = ((double) c.winchCurrent) * nominalWinchCurrent / nominalCommand;//in Ampere


	ds->carouselSpeedSetpoint = ((double) c.carouselSpeedSetpoint) * nominalCarouselSpeed / nominalCommand;//arm speed setpoint in rad/s
	ds->carouselSpeedSmoothed = ((double) c.carouselSpeedSmoothed) * nominalCarouselSpeed / nominalCommand;//(Pseudo-)arm speed in rad/s
	ds->carouselEncoderPosition = ((double) c.carouselEncoderPosition) * 20 * PI / (626074111 - 164054527);//(Pseudo-)arm Position in rad (experimental data)
	ds->carouselTorque = ((double) c.carouselTorque) * nominalCarouselTorque / nominalCommand;//Torque on arm shaft in Nm
	ds->carouselCurrent = ((double) c.carouselCurrent) * nominalCarouselCurrent / nominalCommand;//in Ampere

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

