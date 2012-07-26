#include "tcp2.hpp"
#include <ocl/Component.hpp>
#include <rtt/marsh/Marshalling.hpp>

#define MAX_VALUE_MOTOR_REF 32767
#define CONTROL_SCALE( Value ) \
	(double) Value*1000.0
// Conversion from integer to radians per second
#define	IMU_GYRO_SCALE( Value ) \
        (double)Value / 4.0 * 0.2 * 3.14159 / 180.0
// Conversion from integer to m/s^2
#define IMU_ACCL_SCALE( Value ) \
        (double)Value / 4.0 * 3.333 * 9.81 / 1000.0 * -1.0

ORO_CREATE_COMPONENT( Tcp2)

using namespace std;
using namespace RTT;
using namespace Orocos;

     Tcp2::Tcp2(std::string name) :
		RTT::TaskContext(name)
     {
        //log(Debug) << "(Tcp2) constructor entered" << endlog();

	// Add operations

	addOperation("ReceiveSensorData",&Tcp2::ReceiveSensorData,this).doc("Read sensor data").arg("data","The IMU data will be written to data");
	addOperation("ReceiveSensorDataEmpty",&Tcp2::ReceiveSensorDataEmpty,this).doc("Read sensor data without argument");
	addOperation("SendMotorReferences",&Tcp2::SendMotorReferences,this).doc("Send the motor references");

	// Add ports

	addPort("lastRead",lastRead_port );
	addPort("data",_data);
	addPort("aileron",_aileron);
	addPort( "IMUData_vector",_data1 ).doc("The data from the IMU");
	addPort("IMUCaptureTime",_IMUCaptureTime).doc("Time the measurement is taken (requested)");
	addPort("controlInputPort",_controlInputPort).doc("controlInputPort");
	addPort("controlOutputPort",_controlOutputPort).doc("controlOutputPort: aileron1, aileron2, elevator");
	addPort("delta",_delta).doc("delta");
	addPort("ddelta",_ddelta).doc("ddelta");
	addEventPort("imuTrigger",_imuTrigger, boost::bind(&Tcp2::getIMU,this) ).doc("Trigger the component to get an IMU measurement");
	addEventPort("servoTrigger",_servoTrigger, boost::bind(&Tcp2::getIMU,this) ).doc("Trigger the component to send motor references");
	//addPort("markerPositions",_markerPositions).doc("markerPositions");

	U.resize(4);
	ControlAction.resize(3);


	// Add properties

	addProperty("HostName",hostName).doc("Name to listen for incoming connections on (FQDN or IPv4)");
	addProperty("HostPort",hostPort).doc("Port to listen on (1024-65535 inclusive)");
	addProperty("ConnectionTimeout",connectionTimeout).doc("Timeout in seconds, when waiting for connection");
	addProperty("ReadTimeout",readTimeout).doc("Timeout in seconds, when waiting for read");
        //log(Debug) << "(Tcp2) constructor finished" << endlog();
    }
    

    Tcp2::~Tcp2()
    {
    }

    bool Tcp2::configureHook()
    {
        //log(Debug) << "(Tcp2) configureHook entered" << endlog();
	_aileron.getNewDataOnPortEvent()->connect(boost::bind(&Tcp2::controlAilerons,this));
        //log(Debug) << "(Tcp2) configureHook finished" << endlog();
	return true;
    }

    bool  Tcp2::startHook()
    {
        //log(Debug) << "(Tcp2) startHook entered" << endlog();

	if ( ( g_TCPSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
	{
		printf("Failed to create socket");
		return false;
	}
	// Construct the server sockaddr_in structure
	memset( &echoserver, 0, sizeof( echoserver ) );			// Clear struct
	echoserver.sin_family = AF_INET;						// Internet/IP
	echoserver.sin_addr.s_addr =  inet_addr(hostName.c_str());	// IP address
	echoserver.sin_port = htons( hostPort );       // Server port
	// Establish connection
	if ( connect( g_TCPSocket,
		( struct sockaddr * ) &echoserver,
		sizeof( echoserver ) ) < 0 )
	{
		printf("Failed to connect with server");
		return false;
	}

        //log(Debug) << "(Tcp2) startHook finished" << endlog();
        return true;
    }

    void  Tcp2::updateHook()
    {
	//log(Debug) << "(Tcp2) updateHook entered" << endlog();
		ColumnVector D(6);
		ReceiveSensorData(&D);
		_data.write(D);
		dataV.resize(11);
		dataV[0] = D(1);
		dataV[1] = D(2);
		dataV[2] = D(3);
		dataV[3] = D(4);
		dataV[4] = D(5);
		dataV[5] = D(6);
		dataV[6] = ControlAction[0];
		dataV[7] = ControlAction[1];
		dataV[8] = ControlAction[2];
		_delta.read(delta);
		_ddelta.read(ddelta);
		dataV[9] = delta;
		dataV[10] = ddelta;
		//_markerPositions.read(markerPositions);
		//cout << D(1) << "," << D(2) << "," << D(3) << "," << D(4) << "," << D(5) << "," << D(6) << endl;
		_IMUCaptureTime.write((int) os::TimeService::Instance()->ticks2nsecs(os::TimeService::Instance()->getTicks()));
		_data1.write(dataV);
		_controlInputPort.read(U);
		//SendMotorReferences(CONTROL_SCALE(U[2]),CONTROL_SCALE(U[2]),CONTROL_SCALE(U[3]));
		//SendMotorReferences(CONTROL_SCALE(U[0]),CONTROL_SCALE(U[1]),CONTROL_SCALE(U[2]));
		//SendMotorReferences(U[0],U[1],U[2]);
		//log(Debug) << "(Tcp2) updateHook finished" << endlog();
    }

    void Tcp2::getIMU(){
		ColumnVector D(6);
		ReceiveSensorData(&D);
		_data.write(D);
		dataV.resize(11);
		dataV[0] = D(1);
		dataV[1] = D(2);
		dataV[2] = D(3);
		dataV[3] = D(4);
		dataV[4] = D(5);
		dataV[5] = D(6);
		dataV[6] = ControlAction[0];
		dataV[7] = ControlAction[1];
		dataV[8] = ControlAction[2];
		_delta.read(delta);
		_ddelta.read(ddelta);
		dataV[9] = delta;
		dataV[10] = ddelta;
		//_markerPositions.read(markerPositions);
		//cout << D(1) << "," << D(2) << "," << D(3) << "," << D(4) << "," << D(5) << "," << D(6) << endl;
		_IMUCaptureTime.write((int) os::TimeService::Instance()->ticks2nsecs(os::TimeService::Instance()->getTicks()));
		_data1.write(dataV);
    }

    void Tcp2::controlAilerons()
    {
		double value;
		_aileron.read(value);
		if(value>20.0){value = 20.0;}
		if(value<-20.0){value = -20.0;}
		value = value/20.0*MAX_VALUE_MOTOR_REF;
		SendMotorReferences((int) value,(int) value,(int) value);
    }

    void  Tcp2::stopHook()
    {
        //log(Debug) << "(Tcp2) stopHook entered" << endlog();
	SendMotorReferences(0,0,0);
	close( g_TCPSocket );
        //log(Debug) << "(Tcp2) stopHook finished" << endlog();
    }

    int Tcp2::SendMotorReferences( int m1_ref, int m2_ref, int m3_ref )
    {
	{
	RTT::os::MutexLock lock(m);
		if(m1_ref>MAX_VALUE_MOTOR_REF){m1_ref = MAX_VALUE_MOTOR_REF;}
		if(m2_ref>MAX_VALUE_MOTOR_REF){m2_ref = MAX_VALUE_MOTOR_REF;}
		if(m3_ref>MAX_VALUE_MOTOR_REF){m3_ref = MAX_VALUE_MOTOR_REF;}
		if(m1_ref<-MAX_VALUE_MOTOR_REF){m1_ref = -MAX_VALUE_MOTOR_REF;}
		if(m2_ref<-MAX_VALUE_MOTOR_REF){m2_ref = -MAX_VALUE_MOTOR_REF;}
		if(m3_ref<-MAX_VALUE_MOTOR_REF){m3_ref = -MAX_VALUE_MOTOR_REF;}

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
		/*ControlAction[0] = (double) m1_ref/CONTROL_SCALE(1);
		ControlAction[1] = (double) m2_ref/CONTROL_SCALE(1);
		ControlAction[2] = (double) m3_ref/CONTROL_SCALE(1);*/

		ControlAction[0] =  m1_ref;
		ControlAction[1] =  m2_ref;
		ControlAction[2] =  m3_ref;

		_controlOutputPort.write(ControlAction);
	
		return 0;
	}
    }

    int Tcp2::ReceiveSensorData( ColumnVector *data )
    {
	{
	RTT::os::MutexLock lock(m);
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
		(*data)(1) = g_sIMUSensorDataDouble.XGyro;
		(*data)(2) = g_sIMUSensorDataDouble.YGyro;
		(*data)(3) = g_sIMUSensorDataDouble.ZGyro;
		(*data)(4) = g_sIMUSensorDataDouble.XAccl;
		(*data)(5) = g_sIMUSensorDataDouble.YAccl;
		(*data)(6) = g_sIMUSensorDataDouble.ZAccl;

                //printf("x acceleration: %f \n", g_sIMUSensorDataDouble.XAccl);
                //printf("y acceleration: %f \n", g_sIMUSensorDataDouble.YAccl);
                //printf("z acceleration: %f \n", g_sIMUSensorDataDouble.ZAccl);
		//printf("x rotation: %f \n", g_sIMUSensorDataDouble.XGyro);
		//printf("y rotation: %f \n", g_sIMUSensorDataDouble.YGyro);
		//printf("z rotation: %f \n", g_sIMUSensorDataDouble.ZGyro);
		return 0;
	}
    }
	void Tcp2::ReceiveSensorDataEmpty(){
		//ColumnVector D(6);
		//ReceiveSensorData(&D);
		//_data.write(D);
	}

	int Tcp2::EthernetTransmitReceive( void )
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

