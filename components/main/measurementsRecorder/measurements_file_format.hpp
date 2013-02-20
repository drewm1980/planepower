#ifndef MEASUREMENTS_FILE_FORMAT
#define MEASUREMENTS_FILE_FORMAT

#define NU		3		// number of control inputs
#define NY		24		// number of measurements
#define NY_POSE	12		// # of measurements from pose component / or markers component; in the later case it is number of markers
#define NY_MARK 12
#define NY_IMU	6		// # of IMU measurements
#define NY_ENC	2		// # of encoder measurements, angle true, ang. velocity fake, some averaging; vel. used only for initialization
#define NY_CTRL	3		// # of control measurements; ur1, ur2 and up
#define IMU_VEC_SIZE	(4 * NY_IMU)	// Samples + Cov. matrix diag. elements
#define N_STATE 22

#define BUFFER_IN_MINUTES 30
#define SAMPLING_FREQUENCY 10 // Hz
#define BUFFER_IN_SAMPLES (BUFFER_IN_MINUTES*60*SAMPLING_FREQUENCY)

#define DEFAULT_LOG_FILE_NAME "log.dat"

struct Sample
{
	double measurementsMarkers[NY_MARK*2];
	double measurementsIMU[IMU_VEC_SIZE];  // Samples, plus Cov. Matrix diag. elements, in imuBuffer output format.
	double measurementsEncoder[NY_ENC];
	double measurementsCtrl[NY_CTRL];
	double measurementsCtrlRates[NY_CTRL];
	double measurementsPose[NY_POSE];  // Just the x,y,z,e11,e12,...
	double stateEstimate[N_STATE];  // Just the x,y,z,e11,e12,...
};

#endif
