#ifndef __LISA_BBONE_CLIENT__
#define __LISA_BBONE_CLIENT__

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

#include "types/LisaBboneClientDataType.hpp"

#include "udp_communication.h"
#include "data_decoding.h"

#define MAX_INPUT_STREAM_SIZE  255
#define MAX_OUTPUT_STREAM_SIZE 20

/// Define the time-stamp type
typedef uint64_t TIME_TYPE;

/// LisaBboneClient class
class LisaBboneClient
	: public RTT::TaskContext
{
public:
	/// Ctor
	LisaBboneClient(std::string name);
	
	/// Dtor
	virtual ~LisaBboneClient()
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
	LisaBboneClientDataType data;
	/// The data from the IMU.
	RTT::OutputPort< LisaBboneClientDataType > portData;
	/// Port with control signals [ua1, ua2, ue]. Must be: -1..1
	RTT::InputPort< std::vector< double > > portControls;	
	/// Holder for the control action to be send
	std::vector< double > controls;
	/// Internal buffer for control values
	
	/// Name to listen for incoming connections on, either FQDN or IPv4 address.
	std::string hostName;
	/// Port to listen on: upstream and downstream
	unsigned hostPortUpstream, hostPortDownstream;
	/// Timeout in seconds, when waiting for connection.
	unsigned connectionTimeout;
	/// Timeout in seconds, when waiting to read
	unsigned readTimeout;
	/// Sampling time of the component
	double Ts;
	/// RT mode indicator
	bool rtMode;

private:

	void printStatus();
	void sendReferences();
	void inputDataProcessing();

	UDP_errCode udpStatus;
	DEC_errCode decodingStatus;

	std::vector< double > execControls;

	uint8_t inputStream[ MAX_INPUT_STREAM_SIZE ];
	uint8_t outputStream[ MAX_OUTPUT_STREAM_SIZE ];

	UDP udpUpstream;
	UDP udpDownstream;

	unsigned upTimeCnt;

	Output outputData;
	
};

#endif // __LISA_BBONE_CLIENT__
