#include "mcuHandler.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <pthread.h>

#include <cmath>

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
		(float)Value / 4.0 * 0.2 * M_PI / 180.0

/// Conversion from integer to m/s^2
#define IMU_ACCL_SCALE( Value ) \
		(float)Value / 4.0 * 3.333 * 9.81 / 1000.0 * -1.0

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
	addPort("data", portMcuData)
		.doc("The MCU data");
	addPort("controls", portControls)
		.doc("Port with control signals [ua1, ua2, ue]. Units are in radians.");
	
	//
	// Prepare ports
	//
	portMcuData.setDataSample( data );
	portMcuData.write( data );
	
	controls.reset();
	execControls.reset();

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

#ifdef NONREALTIME_DEBUGGING
	// Provide ability to manually set the flight surfaces, for testing,
	// calibration, etc...
	addOperation("setControlsRadians", &McuHandler::setControlsRadians, this, OwnThread)
		.doc("Set the values for the flight surfaces, in units of Radians")
		.arg("right_aileron", "Right Aileron, in units of Radians, positive is DOWN")
		.arg("left_aileron", "Left Aileron, in units of Radians, positive is Up")
		.arg("elevator", "Elevator, in units of Radians, positive is Up");
	addOperation("setControlsUnitless", &McuHandler::setControlsUnitless, this, OwnThread)
		.doc("Set the values for the flight surfaces, scaled from -1 to 1")
		.arg("right_aileron", "Right Aileron, scaled from -1 to 1, positive is DOWN")
		.arg("left_aileron", "Left Aileron, scaled from -1 to 1, positive is UP")
		.arg("elevator", "Elevator, scaled from -1 to 1, positive is UP");
#endif
}
	
bool McuHandler::configureHook()
{
	pthread_setname_np(pthread_self(), "depl:McuHandler");

	// One of those two has to be positive, depending on the way
	// the component is triggered
	assert(getPeriod() > 0.0 or Ts > 0.0);

	// If the component is indeed configured to be periodic,
	// override whatever was the configuration for Ts.
	if (getPeriod() > 0.0)
		Ts = getPeriod();

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
	
	controls.reset();
	execControls.reset();
	
	sendMotorReferences();

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

	// Read and update controls
	controlStatus = portControls.read( controls );
	updateControls();

	// Before calling the MCU operation, reset the tcpStatus flag.
	tcpStatus = OK;
	sendMotorReferences();

	data.ctrl = execControls;

	data.ts_elapsed = TimeService::Instance()->secondsSince( triggerTimeStamp );
	portMcuData.write( data );
	
	// This is a check for the real-time mode. In case we do not meet the
	// deadline, abort.
	if (rtMode == true && data.ts_elapsed > Ts && (++upTimeCnt > MAX_ERRORS_ALLOWED))
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
	execControls.reset();
	sendMotorReferences();
	
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

void McuHandler::updateControls()
{
	// NOTE: A bit of the logic behind this function... If we are in the derivative
	//       mode, first calculate new controls which are going to be applied. If
	//       new derivatives arrive, they are going to be applied during the next
	//       sampling instance.


//	if (execControls.der_ctrl == true)
//	{
		// Calculate current controls;
		execControls.ua1 += execControls.d_ua1 * Ts;
		execControls.ua2 += execControls.d_ua2 * Ts;
		execControls.ue  += execControls.d_ue  * Ts;
//	}

	if (controlStatus == NewData)
	{
		if (controls.der_ctrl == true)
		{
			execControls.d_ua1 = controls.d_ua1;
			execControls.d_ua2 = controls.d_ua2;
			execControls.d_ue  = controls.d_ue;
			
			execControls.der_ctrl = true;
		}
		else
		{
			execControls.ua1 = controls.ua1;
			execControls.ua2 = controls.ua2;
			execControls.ue  = controls.ue;

			execControls.d_ua1 = execControls.d_ua2 = execControls.d_ue = 0.0;
			
			execControls.der_ctrl = false;
		}
	}
}

void McuHandler::setControlsRadians(float right_aileron, float left_aileron, float elevator)
{
	execControls.ua1 = right_aileron;
	execControls.ua2 = left_aileron;
	execControls.ue  = elevator;

	execControls.der_ctrl = false;
	
	sendMotorReferences();
}
void McuHandler::setControlsUnitless(float right_aileron, float left_aileron, float elevator)
{
	convertControlsUnitlessRadians(right_aileron, left_aileron, elevator);
	execControls.ua1 = right_aileron;
	execControls.ua2 = left_aileron;
	execControls.ue  = elevator;

	execControls.der_ctrl = false;
	
	sendMotorReferences();
}

/// Method for sending the references. All input must be scaled to -1.. +1.
void McuHandler::sendMotorReferences( void )
{
	float ua1 = execControls.ua1;
	float ua2 = execControls.ua2;
	float ue  = execControls.ue;

	// Convert temps to (-1, 1)
	convertControlsRadiansUnitless(ua1, ua2, ue);

	//
	// Clip the control values
	//
	if (ua1 > 1.0) ua1 = 1.0; else if (ua1 < -1.0) ua1 = -1.0;
	if (ua2 > 1.0) ua2 = 1.0; else if (ua2 < -1.0) ua2 = -1.0;
	if (ue  > 1.0) ue  = 1.0; else if (ue  < -1.0) ue  = -1.0;

	// Return back the trimmed controls to exec variable
	convertControlsUnitlessRadians(ua1, ua2, ue);
	execControls.ua1 = ua1;
	execControls.ua2 = ua2;
	execControls.ue  = ue;
	
	//
	// Scale controls
	//
	short execRef1 = (short)(ua1 * MAX_VALUE_MOTOR_REF);
	short execRef2 = (short)(ua2 * MAX_VALUE_MOTOR_REF);
	short execRef3 = (short)(ue  * MAX_VALUE_MOTOR_REF);

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

/// Method for receiving sensor data. Data is NOT scaled, yet.
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
	data.ts_trigger = triggerTimeStamp;
	
	// And now fill in the IMU data
	data.gyro_x = IMU_GYRO_SCALE((short)((receiveBuffer[ 3  ] << 8 ) + receiveBuffer[ 4  ]));
	data.gyro_y = IMU_GYRO_SCALE((short)((receiveBuffer[ 5  ] << 8 ) + receiveBuffer[ 6  ]));
	data.gyro_z = IMU_GYRO_SCALE((short)((receiveBuffer[ 7  ] << 8 ) + receiveBuffer[ 8  ]));

	data.accl_x = IMU_ACCL_SCALE((short)((receiveBuffer[ 9  ] << 8 ) + receiveBuffer[ 10 ]));
	data.accl_y = IMU_ACCL_SCALE((short)((receiveBuffer[ 11 ] << 8 ) + receiveBuffer[ 12 ]));
	data.accl_z = IMU_ACCL_SCALE((short)((receiveBuffer[ 13 ] << 8 ) + receiveBuffer[ 14 ]));

	// TODO Update typekit and include magnetometer data
// 	data.magn_x = ( receiveBuffer[ 15 ] << 8 ) + receiveBuffer[ 16 ];
// 	data.magn_y = ( receiveBuffer[ 17 ] << 8 ) + receiveBuffer[ 18 ];
// 	data.magn_z = ( receiveBuffer[ 19 ] << 8 ) + receiveBuffer[ 20 ];
}

/// Method that communicates with the MCU. In case of ANY error, it triggers
/// the error hook.
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

ORO_LIST_COMPONENT_TYPE( McuHandler )
//ORO_CREATE_COMPONENT( McuHandler )

