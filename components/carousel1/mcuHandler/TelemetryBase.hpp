#ifndef __TELEMETRY_BASE__
#define __TELEMETRY_BASE__

// OROCOS headers
#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

// ZMQ header
#include <components/common/zhelpers.hpp>

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
	/// Error hook.
	virtual void errorHook( );

protected:

	virtual void fill() = 0;

	RTT::InputPort< D > portData;
	D data;

	P protobuf;

	std::string port;

private:

	// TODO make those guys shared pointers
	zmq::context_t* zContext;
	zmq::socket_t*  zSocket;

	std::string raw;
};

TelemetryBase::TelemetryBase(std::name name)
	: RTT::TaskContext(name, PreOperational)
{
	addPort("msgData", portData)
		.doc("Message data");

	addProperty("port", port)
		.doc("Port for publishing the data");

	//
	// Execute a test run to reveal the message size
	// and reserve memory for transport buffer
	//
	fill();
	raw.resize(protobuf.ByteSize(), 0);
}

bool TelemetryBase::configureHook()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	zContext = new zmq::context( 1 );

	if (port.empty() == true)
		return false;

	return true;
}

bool TelemetryBase::startHook()
{
	zSocket = new zmq::socket_t(*zContext, ZMQ_PUB);
	zSocket->bind( port.c_str() );

	return true;
}

void TelemetryBase::updateHook()
{
	if (portData.read( data ) == RTT::NewData)
	{
		// Fill in the protobuf
		fill();
		// Serialize data
		protobuf.SerializeToString( &raw );
		// Transmit serialized data
		if (s_send(*zSocket, raw) == false)
			exception();
	}
}

void TelemetryBase::stopHook()
{
	s_send(*zSocket, raw);

	delete zSocket;
}

void TelemetryBase::cleanupHook()
{
	delete zContext;
}

void TelemetryBase::errorHook()
{}

#endif // __TELEMETRY_BASE__
