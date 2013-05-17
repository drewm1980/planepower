#ifndef __MCUHANDLER__
#define __MCUHANDLER__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Port.hpp>

#include <vector>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>

/// Define the time-stamp type
typedef uint64_t TIME_TYPE;

/// Size of the receive buffer
#define RECEIVE_BUFFER_SIZE	64
/// Size of the transmit buffer
#define TRANSMIT_BUFFER_SIZE	64

/// McuHandler class
class McuHandler : public RTT::TaskContext
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

	/// Method for sending the references. All input must be scaled to -1.. +1.
	void sendMotorReferences(double ref1, double ref2, double ref3);
	
protected:
	
	/// Method that communicates with the MCU. In case of ANY error, it triggers
	/// the error hook.
	void ethernetTransmitReceive( void );
	/// Method for receiving sensor data. Data is NOT scaled, yet.
	void receiveSensorData(std::vector< double >& data);
	
	/// Trigger the component to get IMU data if there is an event on this port.
	RTT::InputPort< TIME_TYPE > portTrigger;
	/// Time stamp of the input trigger.
	TIME_TYPE timeStamp;
	/// Holder for the IMU data.
	std::vector< double > imuData;
	/// The data from the IMU: [timestamp omegax omegay omegaz ax ay az].
	/// All data is normalized.
	RTT::OutputPort< std::vector< double > > portImuData;
	/// Port with control signals [ua1, ua2, ue].
	RTT::InputPort< std::vector< double> > portControls;	
	/// Holder for the control action to be send
	std::vector< double > controls;
	/// Internal buffer for control values
	std::vector< double > execControls;
	/// Port which holds the execution time
	RTT::OutputPort< double > portExecTime;
	
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

	long numOfBytesToBeReceived;
	long numOfBytesToBeTransmitted;
	
	unsigned char receiveBuffer[ RECEIVE_BUFFER_SIZE ];
	unsigned char transmitBuffer[ TRANSMIT_BUFFER_SIZE ];
	
	int tcpSocket;
	struct sockaddr_in tcpEchoServer;
	int tcpStatus;
};

#endif // __MCUHANDLER__
