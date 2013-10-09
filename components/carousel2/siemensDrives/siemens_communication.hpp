#ifndef SIEMENS_COMMUNICATION_HPP
#define SIEMENS_COMMUNICATION_HPP

#include "stdint.h"
#include "udp_communication.hpp"

// This is a high-level API for interfacing with the siemens drives.
// This API is NOT threadsafe, i.e. it should only ever be used from ONE thread!!

// This is a wire format we defined to hold the references for both motors
struct UDPSpeedCommand {
	uint32_t winchSpeedReference;
	uint32_t carouselSpeedReference;
//	uint32_t checksum;
};

struct EncoderState
{
	int64_t bigCounts;
	int32_t smallCountsLast;
};

class Siemens
{
	public:
		Siemens();
		~Siemens();
		int send_reference_speeds(double winch_speed, // in m/s, positive for reelout
				double carousel_speed); // in rad/s, positive for clockwise rotation when viewed from above
		int send_winch_reference_speed(double winch_speed);
		int send_carousel_reference_speed(double carousel_speed);

		// Read the positions and perform any necessary calibration
		// Note:  While the motors are turning, this function should be
		//		  called at least once every 3 hours to avoid missing a 32 bit
		//		  rollover event.
		void read_positions(double *tether_length, // in m, measured from end of arm
							double *cos_delta, // unitless
							double *sin_delta); // unitless
	private:
		EncoderState winch;
		EncoderState carousel;
		void handle_32bit_rollover(EncoderState *e, uint32_t smallCounts);
		void recompute_checksum(UDPSpeedCommand * c);

		UDP udp_client;
		int port_number;
		char ip_address[16];
		
		double currentWinchCommand;
		double currentCarouselCommand;

};

#endif
