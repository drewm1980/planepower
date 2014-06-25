#ifndef ARMBONE_COMMUNICATION_HPP
#define ARMBONE_COMMUNICATION_HPP

#include <stdint.h>

#include "LineAngles.h"

#include "udp_communication.h"
#include "data_decoding.h"

#define MAX_INPUT_STREAM_SIZE 255
#define UDP_SOCKET_TIMEOUT 1000000000


struct UDPReceivePacket {
	int32_t azimuth;
	int32_t elevation;
};

class ArmboneReceiver
{
	public:
		ArmboneReceiver();	
		~ArmboneReceiver();
	
		void read(LineAngles *lineangles);	


	protected:

	private:
		UDP udp_server;
		int port_number;

		uint8_t input_stream[30];
};

#endif
