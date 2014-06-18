#ifndef SIEMENS_DRIVE_STATE
#define SIEMENS_DRIVE_STATE

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

#endif
