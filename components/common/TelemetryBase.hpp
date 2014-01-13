#ifndef __TELEMETRY_BASE__
#define __TELEMETRY_BASE__

// OROCOS headers
#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>

// ZMQ header
#include <zhelpers.hpp>
// Protobuf
#include <google/protobuf/stubs/common.h>

/// A base telemetry class
template< typename D, class P >
class TelemetryBase
  : public RTT::TaskContext
{
public:
	// Ctor
	TelemetryBase(std::string name);

	// Dtor
	~TelemetryBase()
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

protected:

	virtual void fill() = 0;

	RTT::InputPort< D > portData;
	D data;

	P msg;

	std::string port;

private:

	// TODO make those guys shared pointers
	zmq::context_t* zContext;
	zmq::socket_t*  zSocket;

	std::string raw;
};

template< typename D, class P >
TelemetryBase<D, P>::TelemetryBase(std::string name)
  : RTT::TaskContext(name, PreOperational)
{
	addPort("msgData", portData)
		.doc("Message data");

	addProperty("port", port)
	  .doc("Port for publishing the data");

	zContext = NULL;
	zSocket = NULL;
}

template< typename D, class P >
bool TelemetryBase<D, P>::configureHook()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	zContext = new zmq::context_t( 1 );

	//
	// Execute a test run to reveal the message size
	// and reserve memory for transport buffer
	//
	fill();
	raw.resize(msg.ByteSize(), 0);

	if (port.empty() == true)
		return false;

	return true;
}

template< typename D, class P >
bool TelemetryBase<D, P>::startHook()
{
	zSocket = new zmq::socket_t(*zContext, ZMQ_PUB);
	zSocket->bind( port.c_str() );

	return true;
}

template< typename D, class P >
void TelemetryBase<D, P>::updateHook()
{
	if (portData.read( data ) == RTT::NewData)
	{
		// Fill in the protobuf
		fill();
		// Serialize data
		msg.SerializeToString( &raw );
		// Transmit serialized data
		if (s_send(*zSocket, raw) == false)
			exception();
	}
}

template< typename D, class P >
void TelemetryBase<D, P>::stopHook()
{
	if (zSocket != NULL)
		delete zSocket;
}

template< typename D, class P >
void TelemetryBase<D, P>::cleanupHook()
{
  google::protobuf::ShutdownProtobufLibrary();

	if (zContext != NULL)
		delete zContext;
}

#endif // __TELEMETRY_BASE__
