#ifndef __TCP2__
#define __TCP2__

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

#include <newmat/include.h>
#include <newmat/newmat.h>
#include <newmat/newmatap.h>
#include <newmat/newmatio.h>
#include <newmat/newmatnl.h>

#include <math.h>
#include <iostream>
#include <vector>

#include <fstream>
using namespace NEWMAT;

#define RECEIVE_BUFFER_SIZE		64
#define TRANSMIT_BUFFER_SIZE	64

#define TAG_CMD        			0xff

#define CMD_SET_MOTOR_REFS		0x40
#define CMD_GET_SENSOR_DATA		0x41

    /// Tcp2 class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class Tcp2 : public RTT::TaskContext
    {
protected:
	// DATA INTERFACE

	// *** OUTPUTS ***

	/// the last read data
	RTT::OutputPort<std::string>			lastRead_port;
	RTT::OutputPort<ColumnVector>			_data;
	RTT::InputPort<double>				_aileron;
	RTT::OutputPort<std::vector<double> >	_data1;			// The data from the IMU
	RTT::OutputPort<int>	_IMUCaptureTime;			// The time of capture from the IMU
	std::vector<double>			U;
	RTT::InputPort<std::vector<double> >	_controlInputPort;	// The time of capture from the IMU
	RTT::OutputPort<std::vector<double> >	_controlOutputPort;	// The time of capture from the IMU
	std::vector<double>			ControlAction;
	std::vector<double>			dataV;			// Store data internally (std::vector)
	RTT::InputPort<double> _delta;
	RTT::InputPort<double> _ddelta;
	RTT::InputPort<RTT::os::TimeService::ticks>			_imuTrigger;
	RTT::InputPort<RTT::os::TimeService::ticks>			_servoTrigger;
	double delta;
	double ddelta;

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
	RTT::os::Mutex m;
	void getIMU();

public:
	Tcp2(std::string name);
	virtual ~Tcp2();

	int EthernetTransmitReceive( void );
	int ReceiveSensorData( ColumnVector *data );
	void ReceiveSensorDataEmpty( void );
	int SendMotorReferences( int m1_ref, int m2_ref, int m3_ref );
	void controlAilerons();
	int a;
    };
#endif // __TCP2__
