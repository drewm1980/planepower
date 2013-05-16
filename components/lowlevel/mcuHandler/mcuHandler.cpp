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
#define MAX_VALUE_MOTOR_REF 32767

/// Conversion from integer to radians per second
#define	IMU_GYRO_SCALE( Value ) \
		(double)Value / 4.0 * 0.2 * 3.14159 / 180.0

/// Conversion from integer to m/s^2
#define IMU_ACCL_SCALE( Value ) \
		(double)Value / 4.0 * 3.333 * 9.81 / 1000.0 * -1.0

using namespace std;
using namespace RTT;
using namespace RTT::os;

/// MCU handler error codes. ONLY for internal use!
enum McuHandlerErrorCodes
{
	ERR_BAD_COMMAND_RESPONSE = 0,
	ERR_TCP_SEND_FAIL,
	ERR_TCP_RECV_FAIL,
	ERR_BAD_CHECKSUM
};

McuHandler::McuHandler(std::string name)
	: RTT::TaskContext( name )
{
	//
	// Add ports
	//
	addEventPort("trigger", portTrigger)
		.doc("Trigger the component to get an IMU measurement");
	addPort("imuData", portImuData)
		.doc("The IMU data: [timestamp, omegax, omegay, omegaz, ax, ay, az]");
	addPort("controls", portControls)
		.doc("Port with control signals [ua1, ua2, ue].");
	addPort("execTime", portExecTime)
		.doc("Execution time of the component.");

	//
	// Prepare ports
	//
	imuData.resize(7, 0.0);
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
}
	
bool McuHandler::configureHook()
{
	return true;
}

bool McuHandler::startHook()
{
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

	return true;
}

void McuHandler::updateHook()
{
	TimeService::ticks tickStart = TimeService::Instance()->getTicks();
	
	portTrigger.read( timeStamp );
	
	if (portControls.read( controls ) == NewData)
		copy(controls.begin(), controls.end(), execControls.begin());
	
	receiveSensorData( imuData );
	portImuData.write( imuData );
	sendMotorReferences(execControls[ 0 ], execControls[ 1 ], execControls[ 2 ]);
	
	portExecTime.write(
		TimeService::Instance()->secondsSince( tickStart )
	);
}	
	
void McuHandler::stopHook()
{
	/// Try to send neutral references to all motors
	sendMotorReferences(0., 0., 0.);
	/// Close the socket
	close( tcpSocket );
}

void McuHandler::cleanupHook()
{}

void McuHandler::errorHook()
{
	/// Try to send neutral references to all motors
// 	sendMotorReferences(0., 0., 0.);
	/// Close the socket
	close( tcpSocket );
	/// Now print the error
	switch ( tcpStatus )
	{
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
			
		default:
			log( Error ) << "Unknown bug." << endlog();
	};
}

void McuHandler::sendMotorReferences(double ref1, double ref2,double ref3)
{
	//
	// Clip the control values
	//
	if (ref1 > 1.0) ref1 = 1.0; if (ref1 < -1.0) ref1 = -1.0;
	if (ref2 > 1.0) ref2 = 1.0; if (ref2 < -1.0) ref2 = -1.0;
	if (ref3 > 1.0) ref3 = 1.0; if (ref3 < -1.0) ref3 = -1.0;
	
	//
	// Scale controls
	//
	register short execRef1 = (short)(ref1 * MAX_VALUE_MOTOR_REF);
	register short execRef2 = (short)(ref2 * MAX_VALUE_MOTOR_REF);
	register short execRef3 = (short)(ref3 * MAX_VALUE_MOTOR_REF);

	//
	// Prepare the request message
	//
	transmitBuffer[ 0 ] = TAG_CMD;
	transmitBuffer[ 1 ] = 0x0A;
	transmitBuffer[ 2 ] = CMD_SET_MOTOR_REFS;

	transmitBuffer[ 3 ] = (short) execRef1 >> 8;
	transmitBuffer[ 4 ] = (short) execRef1;
	transmitBuffer[ 5 ] = (short) execRef2 >> 8;
	transmitBuffer[ 6 ] = (short) execRef2;
	transmitBuffer[ 7 ] = (short) execRef3 >> 8;
	transmitBuffer[ 8 ] = (short) execRef3;

	numOfBytesToBeTransmitted = transmitBuffer[ 1 ];
	numOfBytesToBeReceived = 0x04;
	
	//
	// Send the message
	//
	ethernetTransmitReceive();
}

void  McuHandler::receiveSensorData(vector< double >& data)
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
		error();
	}
	
	// Timestamp
	data[ 0 ] = (double) TimeService::Instance()->getTicks();
	// And now fill in the IMU data
	data[ 1 ] = IMU_GYRO_SCALE(( receiveBuffer[ 3  ] << 8 ) + receiveBuffer[ 4  ]);
	data[ 2 ] = IMU_GYRO_SCALE(( receiveBuffer[ 5  ] << 8 ) + receiveBuffer[ 6  ]);
	data[ 3 ] = IMU_GYRO_SCALE(( receiveBuffer[ 7  ] << 8 ) + receiveBuffer[ 8  ]);

	data[ 4 ] = IMU_ACCL_SCALE(( receiveBuffer[ 9  ] << 8 ) + receiveBuffer[ 10 ]);
	data[ 5 ] = IMU_ACCL_SCALE(( receiveBuffer[ 11 ] << 8 ) + receiveBuffer[ 12 ]);
	data[ 6 ] = IMU_ACCL_SCALE(( receiveBuffer[ 13 ] << 8 ) + receiveBuffer[ 14 ]);

	// ATM, we do not use the magnetometer data
// 	data[ 7 ] = ( receiveBuffer[ 15 ] << 8 ) + receiveBuffer[ 16 ];
// 	data[ 8 ] = ( receiveBuffer[ 17 ] << 8 ) + receiveBuffer[ 18 ];
// 	data[ 9 ] = ( receiveBuffer[ 19 ] << 8 ) + receiveBuffer[ 20 ];
}

void McuHandler::ethernetTransmitReceive( void )
{
	register int i, j;
	register unsigned long sum;
	register long index;

	// first calculate the checksum...
	for(index = 0, sum = 0; index < (numOfBytesToBeTransmitted - 1); index++)
	{
		sum -= transmitBuffer[ index ];
	}
	transmitBuffer[numOfBytesToBeTransmitted - 1] = sum;

	// then send...
	if (send(tcpSocket, transmitBuffer, numOfBytesToBeTransmitted, 0)
		!= numOfBytesToBeTransmitted )
	{
		tcpStatus = ERR_TCP_SEND_FAIL;
		error();
	}

	// after that receive...
	for (i = 0; i < numOfBytesToBeReceived; )
	{
		j = 0;

		if ((j = recv(tcpSocket, receiveBuffer, numOfBytesToBeReceived, 0)) < 1)
		{
			tcpStatus = ERR_TCP_RECV_FAIL;
			error();
		}

		i += j;
	}

	// calculate the checksum...
	for(index = 0, sum = 0; index < numOfBytesToBeReceived; index++)
	{
		sum += receiveBuffer[ index ];
	}
	if (( sum & 0xFF ) != 0)
	{
		tcpStatus = ERR_BAD_CHECKSUM;
		error();
	}
}

ORO_CREATE_COMPONENT( McuHandler )