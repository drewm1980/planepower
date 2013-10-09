#ifndef __MCUHANDLER__
#define __MCUHANDLER__

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>

#include "types/McuHandlerDataType.hpp"

/// Define the time-stamp type
typedef uint64_t TIME_TYPE;

/// Size of the receive buffer
#define RECEIVE_BUFFER_SIZE	64
/// Size of the transmit buffer
#define TRANSMIT_BUFFER_SIZE	64

/// McuHandler class
class McuHandler
	: public RTT::TaskContext
{
public:
	/// Ctor
	McuHandler(std::string name);
	
	/// Dtor
	virtual ~McuHandler()
	{}
	
	/// Configuration hook.
	virtual bool configureHook( );
	/// Start hook.
	virtual bool startHook( );
	/// Update hook.
	virtual void updateHook( );
	/// Stop hook.
	virtual void stopHook( );
	/// Cleanup hook.
	virtual void cleanupHook( );
	/// Error hook.
	virtual void errorHook( );

protected:

	/// Trigger the component to get IMU data if there is an event on this port.
	RTT::InputPort< TIME_TYPE > portTrigger;
	/// Time stamp of the input trigger. Used in the case when IMU component
	/// is triggered externally.
	TIME_TYPE triggerTimeStamp;
	/// Holder for the IMU data.
	McuHandlerDataType data;
	/// The data from the IMU.
	RTT::OutputPort< McuHandlerDataType > portMcuData;
	/// Port with control signals [ua1, ua2, ue]. Must be: -1..1
	RTT::InputPort< std::vector< double > > portControls;	
	/// Holder for the control action to be send
	std::vector< double > controls;
	/// Internal buffer for control values
	
	/// Name to listen for incoming connections on, either FQDN or IPv4 address.
	std::string hostName;
	/// Port to listen on.
	unsigned hostPort;
	/// Timeout in seconds, when waiting for connection.
	unsigned connectionTimeout;
	/// Timeout in seconds, when waiting to read
	unsigned readTimeout;
	/// Sampling time of the component
	double Ts;
	/// RT mode indicator
	bool rtMode;

private:

	void sendMotorReferences( void );
	void ethernetTransmitReceive( void );
	void receiveSensorData( void );
	
	long numOfBytesToBeReceived;
	long numOfBytesToBeTransmitted;
	
	unsigned char receiveBuffer[ RECEIVE_BUFFER_SIZE ];
	unsigned char transmitBuffer[ TRANSMIT_BUFFER_SIZE ];
	
	int tcpSocket;
	struct sockaddr_in tcpEchoServer;
	int tcpStatus;

	unsigned upTimeCnt;

	std::vector< double > execControls;
};

#endif // __MCUHANDLER__