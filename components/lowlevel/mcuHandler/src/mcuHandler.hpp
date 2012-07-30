#ifndef __MCUHANDLER__
#define __MCUHANDLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <rtt/os/TimeService.hpp>

#include <ocl/OCL.hpp>
#include <string>
#include <fstream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using std::ifstream;

#include <math.h>
#include <iostream>
#include <vector>
#include <unistd.h>

#include <fstream>
using namespace RTT;
using namespace std;

#define RECEIVE_BUFFER_SIZE		64
#define TRANSMIT_BUFFER_SIZE	64

#define TAG_CMD        			0xff

#define CMD_SET_MOTOR_REFS		0x40
#define CMD_GET_SENSOR_DATA		0x41
#include <stdint.h>
typedef uint64_t TIME_TYPE;

    /// McuHandler class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class McuHandler : public RTT::TaskContext
    {
protected:
	// DATA INTERFACE

	// *** OUTPUTS ***

	/// the last read data
	InputPort<TIME_TYPE>					_imuTrigger;		// Trigger the component to get IMU data if there is an event on this port
	vector<double>						imuData;		// Holder for the IMU data
	OutputPort<vector<double> >				_imuData;		// The data from the IMU: [timestamp omegax omegay omegaz ax ay az]
	TIME_TYPE						imuTimeStamp;
	OutputPort<TIME_TYPE>					_imuTimeStamp;
	vector<double>						U;			// Holder for the control action to be send
	vector<double>						U_sent;			// Holder for the control action that were sent
	InputPort<vector<double> >				_controlInputPort;	// The time of capture from the IMU
	OutputPort<vector<double> >				_controlOutputPort;	// The time of capture from the IMU
	OutputPort<TIME_TYPE>					_controlTimeStamp;

	/// the number of items sucessfully read

	// *** CONFIGURATION ***

	// name to listen for incoming connections on, either FQDN or IPv4 addres
	std::string					hostName;
	// port to listen on
	int						hostPort;
	// timeout in seconds, when waiting for connection
	int						connectionTimeout;
	// timeout in seconds, when waiting to read
	int						readTimeout;

	typedef struct TIMUSensorData
	{
		short XGyro;
		short YGyro;
		short ZGyro;
	
		short XAccl;
		short YAccl;
		short ZAccl;
	
		short XMagn;
		short YMagn;
		short ZMagn;	
	} IMUSensorData;

	typedef struct TIMUSensorDataDouble
	{
		double XGyro;
		double YGyro;
		double ZGyro;
	
		double XAccl;
		double YAccl;
		double ZAccl;
	} IMUSensorDataDouble;

	IMUSensorData g_sIMUSensorData;
	IMUSensorDataDouble g_sIMUSensorDataDouble;

	unsigned char g_ucReceiveBuffer[ RECEIVE_BUFFER_SIZE ];
	unsigned char g_ucTransmitBuffer[ TRANSMIT_BUFFER_SIZE ];

	long g_uiNumOfBytesToBeReceived;
	long g_uiNumOfBytesToBeTransmitted;
	int g_TCPSocket;
	struct sockaddr_in echoserver;

	virtual bool configureHook();
	/// reset count and lastRead, attempt to connect to remote
	virtual bool startHook();
	/// attempt to read and process one packet
	virtual void updateHook();
	/// close the socket and cleanup
	virtual void stopHook();

	/// Flag indicating to updateHook() that we want to quit
	bool		quit;

private:
	void getIMUData();
	void sendControls();
	bool useExternalTrigger;

public:
	McuHandler(std::string name);
	virtual ~McuHandler();

	int EthernetTransmitReceive( void );
	int ReceiveSensorData( vector<double> *data );
	int SendMotorReferences( int m1_ref, int m2_ref, int m3_ref );
    };
#endif // __MCUHANDLER__
