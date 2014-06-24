#ifndef SIEMENS_COMMUNICATION_HPP
#define SIEMENS_COMMUNICATION_HPP

#include "stdint.h"
#include "udp_communication.h"
#include "SiemensDriveState.h"
#include "SiemensDriveCommand.h"

// This is a high-level API for interfacing with the siemens drives.
// This API is NOT threadsafe, i.e. it should only ever be used from ONE thread!!

// The following values must be configured to match in the siemens "Starter" app.
#define nominalCommand 0x40000000 // This is the command corresponding to the nominal drive shaft rotation speed, for both drives
#define nominalWinchShaftSpeed 3000.0 // RPM Nominal drive shaft rotation speed when nominalCommand is sent.
									// In starter this called "Reference Speed" or "Rated Motor Speed", p311[0]
									// This can be seen in "Drives -> Servo -> Configuration"
									// This should never be changed!!!
#define nominalCarouselShaftSpeed 1440.0 // RPM Nominal drive shaft rotation speed when nominalCommand is sent.
									// In starter this called "Reference Speed" or "Rated Motor Speed", p311[0]
									// This can be seen in "Drives -> Induction -> Configuration"
									// This should never be changed!!!
					
// ONLY for convenience of typing numbers as percentages manually!
// The limits that matter are implemented in ONE place, in STARTER!
#define winchShaftSpeedAt100 1500.0 // RPM
#define winchSpeedAt100 (winchShaftSpeedAt100*2.0*PI/carouselGearRatio) // m/s
#define carouselSpeedAt100 (2*PI) // rad/s

// Values fixed in hardware
#define carouselGearRatio (14.68*1.5) // input rotations per output rotation.  Includes gears and belt.
#define winchGearRatio 3.0  // input rotations per output rotation
#define winchDrumRadius 0.20 // m

#define nominalCarouselSpeed (nominalCarouselShaftSpeed/carouselGearRatio*2.0*PI/60.0) // rad/s Arm rotation speed when nominalCommand is sent

#define PI 3.14159265359
#define nominalWinchSpeed (nominalWinchShaftSpeed/winchGearRatio*2.0*PI/60.0*winchDrumRadius) // m/s The speed of the surface of the drum when nominalCommand binary value is sent.
#define maxWinchSpeed (maxWinchShaftSpeed/winchGearRatio*2.0*PI/60.0*winchDrumRadius) // m/s The speed of the surface of the drum

#define SIEMENS_DRIVES_SEND_IP_ADDRESS "192.168.000.001"
//The PLC needs to know ~our IP address for this direction of communication

// This is a wire format we defined to hold the references for both motors
struct UDPSendPacket {
	uint32_t winchSpeedReference;
	uint32_t carouselSpeedReference;
};

// This is a wire format defined in the PLC and starter software
struct UDPReceivePacket{
	int32_t winchSpeedSetpoint;
	int32_t winchSpeedSmoothed;
	uint32_t winchEncoderPosition;
	int32_t winchTorque;
	int32_t winchCurrent;
	int32_t carouselSpeedSetpoint;
	int32_t carouselSpeedSmoothed;
	uint32_t carouselEncoderPosition;
	int32_t carouselTorque;
	int32_t carouselCurrent;
};

struct EncoderState
{
	int64_t bigCounts;
	int32_t smallCountsLast;
};

class SiemensSender
{
	public:
		SiemensSender();
		~SiemensSender();
		
		int send_calibrated_speeds(double winch_speed, // in m/s, positive for reelout
				double carousel_speed); // in rad/s, positive for clockwise rotation when viewed from above
		int send_winch_calibrated_speed(double winch_speed); // in m/s
		int send_carousel_calibrated_speed(double carousel_speed); // in rad/s

		int write(SiemensDriveCommand command);

		void stop_drives();

	private:
		// Outside this class, values should be in physical units
		int send_reference_speeds(double winch_speed, // in percentage of nominal value
				double carousel_speed); // in percentage of nominal value
		int send_winch_reference_speed(double winch_speed); // in percentage of nominal value
		int send_carousel_reference_speed(double carousel_speed); // in percentage of nominal value

		UDP udp_client;
		int port_number;
		char ip_address[16];
		
		double currentWinchCommand; 
		double currentCarouselCommand;
};

class SiemensReceiver
{
	public:
		SiemensReceiver();
		~SiemensReceiver();
		
		//// Read a UDP packet from the drives and perform unit conversions.
		////	See the comments for the SiemensDriveState for more info.
		//// Note:  While the motors are turning, this function should be
		////		  called at least once every 3 hours to avoid missing a 32 bit
		////		  rollover event.
		void read(SiemensDriveState* ds); // Reads data into ds

	private:
		void handle_32bit_rollover(EncoderState *e, uint32_t smallCounts);
		//EncoderState winch;
		//EncoderState carousel;
		UDP udp_server;
		int port_number;
};

#endif
