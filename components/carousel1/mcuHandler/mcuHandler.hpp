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

#define NONREALTIME_DEBUGGING 1

/// Define the time-stamp type
typedef uint64_t TIME_TYPE;

/// Size of the receive buffer
#define RECEIVE_BUFFER_SIZE	64
/// Size of the transmit buffer
#define TRANSMIT_BUFFER_SIZE	64

// For these conversion factors,
// angle_radians = (angle_unitless - OFFSET) * SCALE
#define RIGHT_AILERON_SCALE -0.468
#define LEFT_AILERON_SCALE  -0.468
#define ELEVATOR_SCALE 0.78
#define RIGHT_AILERON_OFFSET 0.0
#define LEFT_AILERON_OFFSET 0.0
#define ELEVATOR_OFFSET 0.0

// Convert units from radians to (-1,1)
// angle_radians = (angle_unitless - OFFSET) * SCALE
// angle_radians/SCALE + OFFSET = angle_unitless
void convertControlsRadiansUnitless(float& ua1, float& ua2, float& ue)
{
	ua1 = ua1 / RIGHT_AILERON_SCALE + RIGHT_AILERON_OFFSET;
	ua2 = ua2 / LEFT_AILERON_SCALE + LEFT_AILERON_OFFSET;
	ue  = ue / ELEVATOR_SCALE + ELEVATOR_OFFSET;
}

// Convert units from (-1,1) to radians
// angle_radians = (angle_unitless - OFFSET) * SCALE
void convertControlsUnitlessRadians(float& ua1, float& ua2, float& ue)
{
	ua1 = (ua1 - RIGHT_AILERON_OFFSET) * RIGHT_AILERON_SCALE;
	ua2 = (ua2 - LEFT_AILERON_OFFSET) * LEFT_AILERON_SCALE;
	ue  = (ue - ELEVATOR_OFFSET) * ELEVATOR_SCALE;
}

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

#ifdef NONREALTIME_DEBUGGING
	// Commands for setting the references.
	// Note:  These are intended to be used manually for debugging purposes!
	// For actual control, you should use the provided input port!
	void setControlsRadians(float right_aileron, float left_aileron, float elevator);
	void setControlsUnitless(float right_aileron, float left_aileron, float elevator);
#endif

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
	/// Port with control signals [ua1, ua2, ue, d_ua1, d_ua2, d_ue].
	RTT::InputPort< ControlSurfacesValues > portControls;	
	/// Holder for the control action to be send
	ControlSurfacesValues controls;
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
	void updateControls();
	
	long numOfBytesToBeReceived;
	long numOfBytesToBeTransmitted;
	
	unsigned char receiveBuffer[ RECEIVE_BUFFER_SIZE ];
	unsigned char transmitBuffer[ TRANSMIT_BUFFER_SIZE ];
	
	int tcpSocket;
	struct sockaddr_in tcpEchoServer;
	int tcpStatus;

	unsigned upTimeCnt;

	ControlSurfacesValues execControls;

	RTT::FlowStatus controlStatus;
};

#endif // __MCUHANDLER__
