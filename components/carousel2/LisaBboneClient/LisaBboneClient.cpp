#include "LisaBboneClient.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <cmath>
#include <cstring>

/// Maximum number of transmission errors before we stop the component
#define MAX_ERRORS_ALLOWED 5
/// # servos
#define NUM_SERVOS 7

#define MAX_ABS_CONTROL 9600

#define SCALE_CONTROL( value ) \
	((int16_t)((double)MAX_ABS_CONTROL * value))

#define DEBUG 0

using namespace std;
using namespace RTT;
using namespace RTT::os;

LisaBboneClient::LisaBboneClient(std::string name)
	: RTT::TaskContext(name, PreOperational)
{
	//
	// Add ports
	//
	addEventPort("trigger", portTrigger)
		.doc("Trigger");
	addPort("data", portData)
		.doc("The output data port");
	addPort("controls", portControls)
		.doc("Port with control signals. Valid range for all signals is -1..1");
	
	//
	// Prepare ports
	//
	memset(&data, 0, sizeof( data ));
	portData.setDataSample( data );
	portData.write( data );
	
	controls.resize(NUM_SERVOS, 0.0);
	execControls.resize(NUM_SERVOS, 0.0);

	//
	// Add properties
	//
	addProperty("HostName", hostName)
		.doc("Name to listen for incoming connections on (FQDN or IPv4)");
	addProperty("UpstreamHostPort", hostPortUpstream)
		.doc("Upstream port to listen on (1024-65535 inclusive)");
	addProperty("DownstreamHostPort", hostPortDownstream)
		.doc("Downstream port to listen on (1024-65535 inclusive)");
	addProperty("Ts", Ts)
		.doc("Sampling time");
	rtMode = false;
	addProperty("rtMode", rtMode)
		.doc("Real-time mode of the component.");
}
	
bool LisaBboneClient::configureHook()
{
	return true;
}

bool LisaBboneClient::startHook()
{
	Logger::In in( getName() );

	// Open downstream link
	udpStatus = openUDPServerSocket(&udpDownstream, hostPortDownstream);
	if (udpStatus != UDP_ERR_NONE)
	{
		printStatus();
		return false;
	}
	
	// Initialize message decoding
	init_decoding();

	udpStatus = openUDPClientSocket(&udpUpstream, hostName.c_str(), hostPortUpstream);
	if (udpStatus != UDP_ERR_NONE)
	{
		printStatus();
		return false;
	}

	for (unsigned i = 0; i < NUM_SERVOS; ++i)
		execControls[ 0 ] = 0.;

	upTimeCnt = 0;

	return true;
}

void LisaBboneClient::updateHook()
{
	if (getPeriod() == 0.0)
		portTrigger.read( triggerTimeStamp );
	else
		triggerTimeStamp = TimeService::Instance()->getTicks();

	//
	// Read the new data and do some processing afterwards
	//
	udpStatus = receiveUDPServerData(&udpDownstream, (void *)&inputStream, sizeof( inputStream ));
	if (udpStatus != UDP_ERR_NONE)
		exception();

	inputDataProcessing();

	//
	// Decode data
	//
	decodingStatus = data_update( inputStream );
	if (decodingStatus != DEC_ERR_NONE)
		exception();

	// TODO convert input data
	
	//
	// If _NEW_ controls arrived, send them upstream
	//
#if DEBUG == 0
	if (portControls.read( controls ) == NewData && controls.size() == NUM_SERVOS)
 	{
 		copy(controls.begin(), controls.end(), execControls.begin());

		sendReferences();
 	}
#else

	sendReferences();

#endif // DEBUG ==0

	data.ts_elapsed = TimeService::Instance()->secondsSince( triggerTimeStamp );
	portData.write( data );
	
	// This is a check for the real-time mode. In case we do not meet the
	// deadline, abort.
	if (rtMode == true && data.ts_elapsed > Ts && (++upTimeCnt > MAX_ERRORS_ALLOWED))
	{
		exception();
	}
	else
		upTimeCnt = 0;
}	
	
void LisaBboneClient::stopHook()
{

	/// Try to send neutral references to all motors
//	if (tcpStatus == OK && this->isRunning() == true)
//	for (unsigned i = 0; i < execControls.size(); execControls[ i ] = 0.0, ++i);
//	sendMotorReferences();
	/// Close the socket

	closeUDPServerSocket( &udpDownstream );
	closeUDPClientSocket( &udpUpstream );
	
	printStatus();

	// Recover component if there was an exception
	if (this->inException() == true)
		recover();
}

void LisaBboneClient::cleanupHook()
{}

void LisaBboneClient::errorHook()
{}

void LisaBboneClient::printStatus()
{
	Logger::In in( getName() );

	switch( udpStatus ) 
	{
	case UDP_ERR_NONE:
		break;
	case  UDP_ERR_INET_ATON:
		log( Error ) << "Failed decoding IP address" << endlog();
		break;

	case UDP_ERR_SEND:
		log( Error ) << "Failed sending UDP data" << endlog();
		break;
			
	case UDP_ERR_CLOSE_SOCKET:
		log( Error ) << "Failed closing UDP socket" << endlog();
		break;
			
	case UDP_ERR_OPEN_SOCKET:
		log( Error ) << "Failed inserting UDP socket" << endlog();
		break;
			
	case UDP_ERR_BIND_SOCKET_PORT:
		log( Error ) << "Failed binding port to socket" << endlog();
		break;
			
	case UDP_ERR_RECV:
		log( Error ) << "Failed receiving UDP data" << endlog();
		break;
			
	default:
		log( Error ) << "Undefined UDP error" << endlog();
		break;
	}

	switch( decodingStatus )
	{
	case DEC_ERR_NONE:
		break;

	case  DEC_ERR_START_BYTE:
		log( Error ) << "Start byte is not 0x99" << endlog();
		break;

	case DEC_ERR_CHECKSUM:
		log( Error ) << "Wrong checksum" << endlog();
		break;

	case DEC_ERR_UNKNOWN_BONE_PACKAGE:
		log( Error ) << "Received unknown package from beaglebone" << endlog();
		break;

	case DEC_ERR_UNKNOWN_LISA_PACKAGE:
		log( Error ) << "Received unknown package from lisa" << endlog();
		break;

	case DEC_ERR_UNKNOWN_SENDER:
		log( Error ) << "Received package from unknown sender" << endlog();
		break;

	default:
		log( Error ) << "Undefined decoding error" << endlog();
		break;
	}

	if (upTimeCnt > MAX_ERRORS_ALLOWED)
		log( Error ) << "Timing deadline is not met..." << endlog();
}

void LisaBboneClient::sendReferences()
{

#if DEBUG == 1

	static bool foo = false;
   
	for (unsigned i = 0; i < NUM_SERVOS; ++i)
		if (foo == false)
		{
			execControls[ i ] = 0.5;
			foo = true;
		}
		else
		{
			execControls[ i ] = -0.5;
			foo = false;
		}

#endif

	for (unsigned i = 0; i < NUM_SERVOS; ++i)
		if (execControls[ i ] > 1.0)
			execControls[ i ] = 1.0;
		else if (execControls[ i ] < -1.0)
			execControls[ i ] = -1.0;

	outputData.message.servo_1 = SCALE_CONTROL( execControls[ 0 ] );
	outputData.message.servo_2 = SCALE_CONTROL( execControls[ 1 ] );
	outputData.message.servo_3 = SCALE_CONTROL( execControls[ 2 ] );
	outputData.message.servo_4 = SCALE_CONTROL( execControls[ 3 ] );
	outputData.message.servo_5 = SCALE_CONTROL( execControls[ 4 ] );
	outputData.message.servo_6 = SCALE_CONTROL( execControls[ 5 ] );
	outputData.message.servo_7 = SCALE_CONTROL( execControls[ 6 ] );
	
	decodingStatus = data_encode(outputData.raw, sizeof( outputData.raw ), outputStream, 1, 72);
	if (decodingStatus != DEC_ERR_NONE)
		exception();
	
	udpStatus = sendUDPClientData(&udpUpstream, &outputStream, sizeof( outputStream ));
	if (udpStatus != UDP_ERR_NONE)
		exception();
}

void LisaBboneClient::inputDataProcessing()
{
	switch_read_write();
	Data* dPtr = get_read_pointer();

	if (inputStream[ 3 ] == 203)
	{
		data.gyro_x = (float)dPtr->lisa_plane.imu_gyro_raw.message.gp;
		data.gyro_y = (float)dPtr->lisa_plane.imu_gyro_raw.message.gq;
		data.gyro_z = (float)dPtr->lisa_plane.imu_gyro_raw.message.gr;
	}

	if (inputStream[ 3 ] == 204)
	{
		data.accl_x = (float)dPtr->lisa_plane.imu_accel_raw.message.ax;
		data.accl_y = (float)dPtr->lisa_plane.imu_accel_raw.message.ay;
		data.accl_z = (float)dPtr->lisa_plane.imu_accel_raw.message.az;
	}

	if (inputStream[ 3 ] == 221)
	{
		data.abs_pressure = (float)dPtr->lisa_plane.baro_raw.message.abs;
		data.diff_pressure = (float)dPtr->lisa_plane.baro_raw.message.diff;
	}

	if (inputStream[ 3 ] == 57)
	{
		data.airspeed = dPtr->lisa_plane.airspeed_ets.message.scaled;
	}

	if (inputStream[ 3 ] ==  155)
	{
		data.gps_ecef_x = (float)dPtr->lisa_plane.gps_int.message.ecef_x;
		data.gps_ecef_y = (float)dPtr->lisa_plane.gps_int.message.ecef_y;
		data.gps_ecef_z = (float)dPtr->lisa_plane.gps_int.message.ecef_z;
	}
}

ORO_CREATE_COMPONENT( LisaBboneClient )
