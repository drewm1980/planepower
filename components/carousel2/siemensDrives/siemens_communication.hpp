#ifndef SIEMENS_COMMUNICATION_HPP
#define SIEMENS_COMMUNICATION_HPP

#include "stdint.h"
#include "udp_communication.h"

#define CHECKSUM 0

// This is a high-level API for interfacing with the siemens drives.
// This API is NOT threadsafe, i.e. it should only ever be used from ONE thread!!

// The following values must be configured to match in the siemens "Starter" app.
#define nominalWinchShaftSpeed 3000.0 // RPM Nominal drive shaft rotation speed
#define winchGearRatio 3.0  // input rotations per output rotation
#define winchDrumRadius 0.20 // m
#define PI 3.14159265359
#define nominalWinchSpeed (nominalWinchShaftSpeed/winchGearRatio*2.0*PI/60*winchDrumRadius) // m/s The speed of the surface of the drum
#define nominalCarouselShaftSpeed 1440.0 // RPM Nominal drive shaft rotation speed
#define carouselGearRatio (14.68*1.5) // input rotations per output rotation.  Includes gears and belt.
#define nominalCarouselSpeed (nominalCarouselShaftSpeed/carouselGearRatio*2.0*PI/60) // rad/s Nominal drive shaft rotation speed
#define nominalCommand 0x40000000 // This is the command corresponding to the nominal drive shaft rotation speed

// This is a wire format we defined to hold the references for both motors
struct UDPSendPacket {
	uint32_t winchSpeedReference;
	uint32_t carouselSpeedReference;
#if CHECKSUM
	uint32_t checksum;
#endif
};

// This is a wire format defined in the PLC and starter software
struct UDPReceivePacket{
	int32_t winchSpeedSmoothed;
	uint32_t winchEncoderPosition;
	int32_t carouselSpeedSmoothed;
	uint32_t carouselEncoderPosition;
	int32_t winchTorque;
	int32_t winchPower;
	int32_t winchSpeedSetpoint;
	int32_t carouselTorque;
	int32_t carouselPower;
	int32_t carouselSpeedSetpoint;
#if CHECKSUM
	uint32_t checksum;
#endif
};

// This is a unit converted version of UDPReceivePacket
struct SiemensDriveState{
	double winchSpeedSmoothed;
	double winchEncoderPosition;
	double carouselSpeedSmoothed;
	double carouselEncoderPosition;
	double winchTorque;
	double winchPower;
	double winchSpeedSetpoint;
	double carouselTorque;
	double carouselPower;
	double carouselSpeedSetpoint;
};

struct EncoderState
{
	int64_t bigCounts;
	int32_t smallCountsLast;
};

class SiemensCommunicator
{
	public:
		SiemensCommunicator();
		~SiemensCommunicator();
		int send_reference_speeds(double winch_speed, // in m/s, positive for reelout
				double carousel_speed); // in rad/s, positive for clockwise rotation when viewed from above
		int send_winch_reference_speed(double winch_speed);
		int send_carousel_reference_speed(double carousel_speed);

		//// Read a UDP packet from the drives and perform unit conversions.
		////	See the comments for the SiemensDriveState for more info.
		//// Note:  While the motors are turning, this function should be
		////		  called at least once every 3 hours to avoid missing a 32 bit
		////		  rollover event.
		void read(SiemensDriveState* ds);

	private:
		EncoderState winch;
		EncoderState carousel;
		void handle_32bit_rollover(EncoderState *e, uint32_t smallCounts);

		UDP udp_client;
		UDP udp_server;
		int port_number;
		char ip_address[16];
		
		double currentWinchCommand;
		double currentCarouselCommand;

};

#endif
