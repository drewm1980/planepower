#include "mcuHandler.hpp"
#include <ocl/Component.hpp>
#include <rtt/marsh/Marshalling.hpp>

#define MAX_VALUE_MOTOR_REF 32767
#define MAX_VALUE_AILERON_REF MAX_VALUE_MOTOR_REF
#define MAX_VALUE_ELEVATOR_REF 20000
//#define MAX_VALUE_ELEVATOR_REF MAX_VALUE_MOTOR_REF
#define CONTROL_SCALE( Value ) \
	(double) Value*1000.0
// Conversion from integer to radians per second
#define	IMU_GYRO_SCALE( Value ) \
		(double)Value / 4.0 * 0.2 * 3.14159 / 180.0
// Conversion from integer to m/s^2
#define IMU_ACCL_SCALE( Value ) \
		(double)Value / 4.0 * 3.333 * 9.81 / 1000.0 * -1.0

ORO_CREATE_COMPONENT( McuHandler)

using namespace std;
using namespace RTT;
using namespace Orocos;

	 McuHandler::McuHandler(std::string name) :
		RTT::TaskContext(name)
	 {

	// Add ports
	addEventPort("imuTrigger",_imuTrigger, boost::bind(&McuHandler::getIMUData,this) ).doc("Trigger the component to get an IMU measurement");
	addPort("imuData",_imuData).doc("The IMU data: [omegax; omegay; omegaz; ax; ay; az]");
	addEventPort("controlInputPort",_controlInputPort,boost::bind(&McuHandler::sendControls,this)).doc("Control input: motor1, motor2, motor3");
	addPort("imuTimeStamp",_imuTimeStamp).doc("The IMU time stamp");
	addPort("controlOutputPort",_controlOutputPort).doc("Control output: motor1, motor2, motor3");
	addPort("controlTimeStamp",_controlTimeStamp);

	U.resize(3);
	U_sent.resize(3,0.0);
	_controlOutputPort.setDataSample(U_sent);
	_controlOutputPort.write(U_sent);	
	imuData.resize(6);

	// Add properties

	addProperty("HostName",hostName).doc("Name to listen for incoming connections on (FQDN or IPv4)");
	addProperty("HostPort",hostPort).doc("Port to listen on (1024-65535 inclusive)");
	addProperty("ConnectionTimeout",connectionTimeout).doc("Timeout in seconds, when waiting for connection");
	addProperty("ReadTimeout",readTimeout).doc("Timeout in seconds, when waiting for read");
	addProperty("useExternalTrigger",useExternalTrigger).doc("Set to true of you want the component to be triggered automatically. Default value is false (meaning triggered by orocos)");
	useExternalTrigger = false;

	addOperation("sendMotorReferences",&McuHandler::SendMotorReferences,this);
	}
	

	McuHandler::~McuHandler()
	{
	}

	bool McuHandler::configureHook()
	{
	return true;
	}

	bool  McuHandler::startHook()
	{
	if ( ( g_TCPSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
	{
		printf("Failed to create socket");
		return false;
	}
	// Construct the server sockaddr_in structure
	memset( &echoserver, 0, sizeof( echoserver ) );			// Clear struct
	echoserver.sin_family = AF_INET;						// Internet/IP
	echoserver.sin_addr.s_addr =  inet_addr(hostName.c_str());	// IP address
	echoserver.sin_port = htons( hostPort );	   // Server port
	// Establish connection
	if ( connect( g_TCPSocket,
		( struct sockaddr * ) &echoserver,
		sizeof( echoserver ) ) < 0 )
	{
		printf("Failed to connect with server");
		return false;
	}

	SendMotorReferences(0,0,0);

		return true;
	}

	void  McuHandler::updateHook()
	{
		if(!useExternalTrigger){
			imuTimeStamp = RTT::os::TimeService::Instance()->getTicks();
			ReceiveSensorData(&imuData);
			_imuData.write(imuData);
			_imuTimeStamp.write(imuTimeStamp);
		}
	}

	void McuHandler::getIMUData(){
		if(useExternalTrigger){
//RTT::os::TimeService::ticks myticks = RTT::os::TimeService::Instance()->getTicks();
			_imuTrigger.read(imuTimeStamp);
//cout << "time it took from request: " << RTT::os::TimeService::Instance()->secondsSince(imuTimeStamp)*1000000.0<< " microseconds." << endl;
			ReceiveSensorData(&imuData);
			_imuTimeStamp.write((TIME_TYPE) imuTimeStamp);
			_imuData.write(imuData);
//cout << "time it took: " << RTT::os::TimeService::Instance()->ticksSince(myticks)/1000.0 << " microseconds." << endl;
		} 
	}

	void McuHandler::sendControls(){
		_controlInputPort.read(U);
		SendMotorReferences((int) U[0], (int) U[1], (int) U[2] );
	}

	void  McuHandler::stopHook()
	{
	SendMotorReferences(0,0,0);
	close( g_TCPSocket );
	}

	int McuHandler::SendMotorReferences( int m1_ref, int m2_ref, int m3_ref )
	{
		if(m1_ref>MAX_VALUE_AILERON_REF){m1_ref = MAX_VALUE_AILERON_REF;cout << "max reached" << endl;}
		if(m2_ref>MAX_VALUE_AILERON_REF){m2_ref = MAX_VALUE_AILERON_REF;cout << "max reached" << endl;}
		if(m3_ref>MAX_VALUE_ELEVATOR_REF){m3_ref = MAX_VALUE_ELEVATOR_REF;cout << "el" << endl;}
		if(m1_ref<-MAX_VALUE_AILERON_REF){m1_ref = -MAX_VALUE_AILERON_REF;cout << "max reached" << endl;}
		if(m2_ref<-MAX_VALUE_AILERON_REF){m2_ref = -MAX_VALUE_AILERON_REF;cout << "max reached" << endl;}
		if(m3_ref<-MAX_VALUE_ELEVATOR_REF){m3_ref = -MAX_VALUE_ELEVATOR_REF;cout << "el" << endl;}

		//
		// Prepare the request message
		//
		g_ucTransmitBuffer[ 0 ] = TAG_CMD;
		g_ucTransmitBuffer[ 1 ] = 0x0A;
		g_ucTransmitBuffer[ 2 ] = CMD_SET_MOTOR_REFS;

		g_ucTransmitBuffer[ 3 ] = (short) m1_ref >> 8;
		g_ucTransmitBuffer[ 4 ] = (short) m1_ref;
		g_ucTransmitBuffer[ 5 ] = (short) m2_ref >> 8;
		g_ucTransmitBuffer[ 6 ] = (short) m2_ref;
		g_ucTransmitBuffer[ 7 ] = (short) m3_ref >> 8;
		g_ucTransmitBuffer[ 8 ] = (short) m3_ref;

		g_uiNumOfBytesToBeTransmitted = g_ucTransmitBuffer[ 1 ];
		g_uiNumOfBytesToBeReceived = 0x04;

		//
		// Transmit the request and wait for the response
		//
		if ( EthernetTransmitReceive() != 0 )
		{
			printf( "Sending of the motor references failed\n" );		
			return 1;
		}
		U_sent[0] = (double) m1_ref;
		U_sent[1] = (double) m2_ref;
		U_sent[2] = (double) m3_ref;
		_imuTrigger.read(imuTimeStamp);
		_controlOutputPort.write(U_sent);
		_controlTimeStamp.write(imuTimeStamp);
	
		return 0;
	}

	int McuHandler::ReceiveSensorData( vector<double> *data )
	{
		//
		// Prepare the request message
		//
		g_ucTransmitBuffer[ 0 ] = TAG_CMD;
		g_ucTransmitBuffer[ 1 ] = 0x04;
		g_ucTransmitBuffer[ 2 ] = CMD_GET_SENSOR_DATA;

		g_uiNumOfBytesToBeTransmitted = g_ucTransmitBuffer[ 1 ];
		g_uiNumOfBytesToBeReceived = 0x16;

		//
		// Transmit the request and wait for the response
		//
		if ( EthernetTransmitReceive() != 0 )
		{
			printf( "Sending the command for receiving the sensor data failed\n" );		
			return 1;
		}

		//
		// Process the response
		//

		if ( g_ucReceiveBuffer[ 2 ] !=  CMD_GET_SENSOR_DATA )
		{
			printf( "Bad command response!\n" );
		} 
	
		g_sIMUSensorData.XGyro = ( g_ucReceiveBuffer[ 3  ] << 8 ) + g_ucReceiveBuffer[ 4  ];
		g_sIMUSensorData.YGyro = ( g_ucReceiveBuffer[ 5  ] << 8 ) + g_ucReceiveBuffer[ 6  ];
		g_sIMUSensorData.ZGyro = ( g_ucReceiveBuffer[ 7  ] << 8 ) + g_ucReceiveBuffer[ 8  ];

		g_sIMUSensorData.XAccl = ( g_ucReceiveBuffer[ 9  ] << 8 ) + g_ucReceiveBuffer[ 10 ];
		g_sIMUSensorData.YAccl = ( g_ucReceiveBuffer[ 11 ] << 8 ) + g_ucReceiveBuffer[ 12 ];
		g_sIMUSensorData.ZAccl = ( g_ucReceiveBuffer[ 13 ] << 8 ) + g_ucReceiveBuffer[ 14 ];

		g_sIMUSensorData.XMagn = ( g_ucReceiveBuffer[ 15 ] << 8 ) + g_ucReceiveBuffer[ 16 ];
		g_sIMUSensorData.YMagn = ( g_ucReceiveBuffer[ 17 ] << 8 ) + g_ucReceiveBuffer[ 18 ];
		g_sIMUSensorData.ZMagn = ( g_ucReceiveBuffer[ 19 ] << 8 ) + g_ucReceiveBuffer[ 20 ];
		
		g_sIMUSensorDataDouble.XGyro = IMU_GYRO_SCALE( g_sIMUSensorData.XGyro );
		g_sIMUSensorDataDouble.YGyro = IMU_GYRO_SCALE( g_sIMUSensorData.YGyro );
		g_sIMUSensorDataDouble.ZGyro = IMU_GYRO_SCALE( g_sIMUSensorData.ZGyro );

		g_sIMUSensorDataDouble.XAccl = IMU_ACCL_SCALE( g_sIMUSensorData.XAccl );
		g_sIMUSensorDataDouble.YAccl = IMU_ACCL_SCALE( g_sIMUSensorData.YAccl );
		g_sIMUSensorDataDouble.ZAccl = IMU_ACCL_SCALE( g_sIMUSensorData.ZAccl );
		(*data)[0] = g_sIMUSensorDataDouble.XGyro;
		(*data)[1] = g_sIMUSensorDataDouble.YGyro;
		(*data)[2] = g_sIMUSensorDataDouble.ZGyro;
		(*data)[3] = g_sIMUSensorDataDouble.XAccl;
		(*data)[4] = g_sIMUSensorDataDouble.YAccl;
		(*data)[5] = g_sIMUSensorDataDouble.ZAccl;

				//printf("x acceleration: %f \n", g_sIMUSensorDataDouble.XAccl);
				//printf("y acceleration: %f \n", g_sIMUSensorDataDouble.YAccl);
				//printf("z acceleration: %f \n", g_sIMUSensorDataDouble.ZAccl);
		//printf("x rotation: %f \n", g_sIMUSensorDataDouble.XGyro);
		//printf("y rotation: %f \n", g_sIMUSensorDataDouble.YGyro);
		//printf("z rotation: %f \n", g_sIMUSensorDataDouble.ZGyro);
		return 0;
	}

	int McuHandler::EthernetTransmitReceive( void )
	{
		static int i, j;
		static unsigned long sum;
		static long index;

		// first calculate the checksum...
		for( index = 0, sum = 0; index < ( g_uiNumOfBytesToBeTransmitted - 1 ); index++ )
		{
		sum -= g_ucTransmitBuffer[ index ];
		}
		g_ucTransmitBuffer[ g_uiNumOfBytesToBeTransmitted - 1 ] = sum;

		// then send...
		if ( send( g_TCPSocket, g_ucTransmitBuffer, g_uiNumOfBytesToBeTransmitted, 0 )
			!= g_uiNumOfBytesToBeTransmitted )
		{
			printf( "TCP sending failed\n" );		
			return 1;
		}

		// after that receive...
		for ( i = 0; i < g_uiNumOfBytesToBeReceived; )
		{
			j = 0;

			if ( ( j = recv( g_TCPSocket, g_ucReceiveBuffer, g_uiNumOfBytesToBeReceived, 0 ) ) < 1 )
			{
				return 1;
				printf( "Failed to receive bytes from server\n" );
			}

			i += j;
		}

		// calculate the checksum...
		for( index = 0, sum = 0; index < g_uiNumOfBytesToBeReceived; index++ )
		{
			sum += g_ucReceiveBuffer[ index ];
		}
		if ( ( sum & 0xFF ) != 0 )
		{
			printf( "Checksum of received data package is bad\n" );		
			return 1;
		}
	
		return 0;
	}

