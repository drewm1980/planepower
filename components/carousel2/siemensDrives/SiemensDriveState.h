#ifndef SIEMENS_DRIVE_STATE
#define SIEMENS_DRIVE_STATE

// This is a unit converted version of UDPReceivePacket,
// plus a couple fields for timestamps
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

	double ts_trigger;
	double ts_elapsed;
};

#endif
