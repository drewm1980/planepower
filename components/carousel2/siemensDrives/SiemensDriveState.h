#ifndef SIEMENS_DRIVE_STATE
#define SIEMENS_DRIVE_STATE

// This is a unit converted version of UDPReceivePacket,
// plus a couple fields for timestamps
struct SiemensDriveState{
	double winchSpeedSetpoint;
	double winchSpeedSmoothed;
	double winchEncoderPosition;
	double winchTorque;
	//double winchPower;
	double carouselSpeedSetpoint;
	double carouselSpeedSmoothed;
	double carouselEncoderPosition;
	double carouselTorque;
	//double carouselPower;

	double ts_trigger;
	double ts_elapsed;
};

#endif
