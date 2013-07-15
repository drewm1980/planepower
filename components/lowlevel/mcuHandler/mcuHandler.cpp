#include "mcuHandler.hpp"

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

/// Packet tag filed
#define TAG_CMD        			0xff
/// Packet identifier for sending motor references.
#define CMD_SET_MOTOR_REFS		0x40
/// Packet identifier for receiving sensor data.
#define CMD_GET_SENSOR_DATA		0x41
/// Max motor reference value (+- max unsigned 16bit number).
#define MAX_VALUE_MOTOR_REF 32767.0

/// Conversion from integer to radians per second
#define	IMU_GYRO_SCALE( Value ) \
		(double)Value / 4.0 * 0.2 * 3.14159 / 180.0

/// Conversion from integer to m/s^2
#define IMU_ACCL_SCALE( Value ) \
		(double)Value / 4.0 * 3.333 * 9.81 / 1000.0 * -1.0

/// Maximum number of transmission errors before we stop the component
#define MAX_ERRORS_ALLOWED 5

using namespace std;
using namespace RTT;
using namespace RTT::os;

/// MCU handler error codes. ONLY for internal use!
enum McuHandlerErrorCodes
{
	OK = 0,
	ERR_BAD_COMMAND_RESPONSE,
	ERR_TCP_SEND_FAIL,
	ERR_TCP_RECV_FAIL,
	ERR_BAD_CHECKSUM,
	ERR_BAD_DATA_SIZE,
	ERR_DEADLINE
};

McuHandler::McuHandler(std::string name)
	: RTT::TaskContext(name, PreOperational)
{
	//
	// Add ports
	//
	addEventPort("trigger", portTrigger)
		.doc("Trigger the component to get an IMU measurement");
	addPort("imuData", portImuData)
		.doc("The IMU data");
	addPort("controls", portControls)
		.doc("Port with control signals [ua1, ua2, ue].");
	
	//
	// Add operations
	//

	// This guy is supposed to be used only for debugging purposes
	this->provides()->addOperation( "sendMotorReferences", &McuHandler::sendMotorReferences, this)
		.doc( "MHE RTI feedback step." );

	//
	// Prepare ports
	//
	portImuData.setDataSample( imuData );
	portImuData.write( imuData );
	
	controls.resize(3, 0.0);
	execControls.resize(3, 0.0);

	//
	// Add properties
	//
	addProperty("HostName", hostName)
		.doc("Name to listen for incoming connections on (FQDN or IPv4)");
	addProperty("HostPort", hostPort)
		.doc("Port to listen on (1024-65535 inclusive)");
	addProperty("ConnectionTimeout", connectionTimeout)
		.doc("Timeout in seconds, when waiting for connection");
	addProperty("ReadTimeout", readTimeout)
		.doc("Timeout in seconds, when waiting for read");
	addProperty("Ts", Ts)
		.doc("Sampling time");
	rtMode = false;
	addProperty("rtMode", rtMode)
		.doc("Real-time mode of the component.");
}
	
bool McuHandler::configureHook()
{
	return true;
}

bool McuHandler::startHook()
{
	Logger::In in( getName() );

	if ((tcpSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		log( Error ) << "Failed to create socket." << endlog();
		return false;
	}
	
   	// Clear struct
	memset(&tcpEchoServer, 0, sizeof( tcpEchoServer ));
	// Internet/IP
	tcpEchoServer.sin_family = AF_INET;
	// IP address
	tcpEchoServer.sin_addr.s_addr = inet_addr(hostName.c_str());
	// Server port
	tcpEchoServer.sin_port = htons( hostPort );
	// Establish connection
	if (connect(tcpSocket, (struct sockaddr *) &tcpEchoServer, sizeof( tcpEchoServer )) < 0)
	{
		log( Error ) << "Failed to connect with the MCU." << endlog();
		return false;
	}

	execControls[ 0 ] = 0.;
	execControls[ 1 ] = 0.;
	execControls[ 2 ] = 0.;
	sendMotorReferences(execControls[ 0 ], execControls[ 1 ], execControls[ 2 ]);

	upTimeCnt = 0;

	return true;
}

void McuHandler::updateHook()
{
	if (getPeriod() == 0.0)
		portTrigger.read( triggerTimeStamp );
	else
		triggerTimeStamp = TimeService::Instance()->getTicks();

	//
	// Read the IMU data, process the packet and output the data.
	//

	// Before calling the MCU operation, reset the tcpStatus flag.
	tcpStatus = OK;
	receiveSensorData();
	imuData.ts_elapsed = TimeService::Instance()->secondsSince( triggerTimeStamp );
	portImuData.write( imuData );

	//
	// If some controls arrived, send them to the MCU
	//
	// TODO this is probably not the nicest solution, consider to remove the
	// "rtMode == true" and sendMotorReferences access from outside, i.e.
	// make the component fully I/O port based.
	//

	if (portControls.read( controls ) == NewData && rtMode == true)
 	{
		// In case we received new commands and we are in the real-time mode
		// we can send new commands to the plane if some checks are satisfied
 		if (controls.size() != 3)
 		{
 			tcpStatus = ERR_BAD_DATA_SIZE;
 			exception();
 		}
 		copy(controls.begin(), controls.end(), execControls.begin());
		
		// Before calling the MCU operation, reset the tcpStatus flag.
		tcpStatus = OK;
		sendMotorReferences(execControls[ 0 ], execControls[ 1 ], execControls[ 2 ]);
 	}

	// This is a check for the real-time mode. In case we do not meet the
	// deadline, abort.
	if (rtMode == true && imuData.ts_elapsed > Ts && (++upTimeCnt > MAX_ERRORS_ALLOWED))
	{
		tcpStatus = ERR_DEADLINE;
		exception();
	}
	else
		upTimeCnt = 0;
}	
	
void McuHandler::stopHook()
{
	Logger::In in( getName() );

	/// Try to send neutral references to all motors
//	if (tcpStatus == OK && this->isRunning() == true) 
	sendMotorReferences(0., 0., 0.);
	/// Close the socket
	close( tcpSocket );
	/// Now print the error
	switch ( tcpStatus )
	{
	case OK:
		break;

	case ERR_BAD_COMMAND_RESPONSE:
		log( Error ) << "Bad command response received." << endlog();
		break;
			
	case ERR_TCP_RECV_FAIL:
		log( Error ) << "Error experienced while receiving data from the MCU." << endlog();
		break;
			
	case ERR_TCP_SEND_FAIL:
		log( Error ) << "Error experienced while sending data to the MCU." << endlog();
		break;
			
	case ERR_BAD_CHECKSUM:
		log( Error ) << "Bad checksum of the received packet from the MCU." << endlog();
		break;

	case ERR_BAD_DATA_SIZE: 
		log( Error ) << "Bad data size." << endlog();
		break;

	case ERR_DEADLINE: 
		log( Error ) << "Exectuion time of this component is longer than the sampling time." << endlog();
		break;

	default:
		log( Error ) << "Unknown bug." << endlog();
	};

	// Recover component if there was an exception
	if (this->inException() == true)
		recover();
}

void McuHandler::cleanupHook()
{}

void McuHandler::errorHook()
{}

void McuHandler::sendMotorReferences(double ref1, double ref2, double ref3)
{
	//
	// Clip the control values
	//
	if (ref1 > 1.0) ref1 = 1.0; else if (ref1 < -1.0) ref1 = -1.0;
	if (ref2 > 1.0) ref2 = 1.0; else if (ref2 < -1.0) ref2 = -1.0;
	if (ref3 > 1.0) ref3 = 1.0; else if (ref3 < -1.0) ref3 = -1.0;
	
	//
	// Scale controls
	//
	short execRef1 = (short)(ref1 * MAX_VALUE_MOTOR_REF);
	short execRef2 = (short)(ref2 * MAX_VALUE_MOTOR_REF);
	short execRef3 = (short)(ref3 * MAX_VALUE_MOTOR_REF);

	//
	// Prepare the request message
	//
	transmitBuffer[ 0 ] = TAG_CMD;
	transmitBuffer[ 1 ] = 0x0A;
	transmitBuffer[ 2 ] = CMD_SET_MOTOR_REFS;

	transmitBuffer[ 3 ] = execRef1 >> 8;
	transmitBuffer[ 4 ] = execRef1;
	transmitBuffer[ 5 ] = execRef2 >> 8;
	transmitBuffer[ 6 ] = execRef2;
	transmitBuffer[ 7 ] = execRef3 >> 8;
	transmitBuffer[ 8 ] = execRef3;

	numOfBytesToBeTransmitted = transmitBuffer[ 1 ];
	numOfBytesToBeReceived = 0x04;
	
	//
	// Send the message
	//
	ethernetTransmitReceive();
}

void  McuHandler::receiveSensorData( void )
{
	//
	// Prepare the request message
	//
	transmitBuffer[ 0 ] = TAG_CMD;
	transmitBuffer[ 1 ] = 0x04;
	transmitBuffer[ 2 ] = CMD_GET_SENSOR_DATA;

	numOfBytesToBeTransmitted = transmitBuffer[ 1 ];
	numOfBytesToBeReceived = 0x16;
	
	//
	// Send the message
	//
	ethernetTransmitReceive();

	//
	// Process the response
	//
	if (receiveBuffer[ 2 ] != CMD_GET_SENSOR_DATA)
	{
		tcpStatus = ERR_BAD_COMMAND_RESPONSE;
		exception();
	}
	
	// Timestamp
	imuData.ts_trigger = triggerTimeStamp;
	
	// And now fill in the IMU data
	imuData.gyro_x = IMU_GYRO_SCALE((short)((receiveBuffer[ 3  ] << 8 ) + receiveBuffer[ 4  ]));
	imuData.gyro_y = IMU_GYRO_SCALE((short)((receiveBuffer[ 5  ] << 8 ) + receiveBuffer[ 6  ]));
	imuData.gyro_z = IMU_GYRO_SCALE((short)((receiveBuffer[ 7  ] << 8 ) + receiveBuffer[ 8  ]));

	imuData.accl_x = IMU_ACCL_SCALE((short)((receiveBuffer[ 9  ] << 8 ) + receiveBuffer[ 10 ]));
	imuData.accl_y = IMU_ACCL_SCALE((short)((receiveBuffer[ 11 ] << 8 ) + receiveBuffer[ 12 ]));
	imuData.accl_z = IMU_ACCL_SCALE((short)((receiveBuffer[ 13 ] << 8 ) + receiveBuffer[ 14 ]));

	// TODO Update typekit and include magnetometer data
// 	imuData.magn_x = ( receiveBuffer[ 15 ] << 8 ) + receiveBuffer[ 16 ];
// 	imuData.magn_y = ( receiveBuffer[ 17 ] << 8 ) + receiveBuffer[ 18 ];
// 	imuData.magn_z = ( receiveBuffer[ 19 ] << 8 ) + receiveBuffer[ 20 ];
}

void McuHandler::ethernetTransmitReceive( void )
{
	int i, j;
	unsigned long sum;
	long index;
	static unsigned numErrors = 0;

	// tcpStatus: exceptions are triggered only in case the current state is 
	// OK. This is implemented in such a way to be able to send last resort
	// signal to the MCU to e.g. reset all position references to zero. In 
	// that case, we do not care to much about what is going to happen, but 
	// to try to do this actually.

	// First calculate the checksum...
	for(index = 0, sum = 0; index < (numOfBytesToBeTransmitted - 1); index++)
	{
		sum -= transmitBuffer[ index ];
	}
	transmitBuffer[numOfBytesToBeTransmitted - 1] = sum;

	// Then send...
	if (send(tcpSocket, transmitBuffer, numOfBytesToBeTransmitted, 0)
		!= numOfBytesToBeTransmitted && tcpStatus == OK)
	{
		tcpStatus = ERR_TCP_SEND_FAIL;
		exception();
	}

	// After that receive...
	for (i = 0; i < numOfBytesToBeReceived; )
	{
		j = 0;

		if ((j = recv(tcpSocket, receiveBuffer, numOfBytesToBeReceived, 0)) < 1 && tcpStatus == OK)
		{
			tcpStatus = ERR_TCP_RECV_FAIL;
			exception();
		}

		i += j;
	}

	// Calculate the checksum...
	for(index = 0, sum = 0; index < numOfBytesToBeReceived; index++)
	{
		sum += receiveBuffer[ index ];
	}
	if (( sum & 0xFF ) != 0 && tcpStatus == OK && (++numErrors > MAX_ERRORS_ALLOWED))
	{
		tcpStatus = ERR_BAD_CHECKSUM;
		exception();
	}
	else
		numErrors = 0; 
}

ORO_CREATE_COMPONENT( McuHandler )
