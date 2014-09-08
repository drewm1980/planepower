#include "kalmanFilter.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

#include <Eigen/Dense>
using Eigen::Matrix;

KalmanFilter::KalmanFilter(std::string name):TaskContext(name,PreOperational) 
{
	addPort("lineAngles",portLineAngles).doc("Line Angle Sensor Measurements");
	addPort("reference",portReference).doc("The reference state");

	addPort("gainsIn",portGansIn).doc("Where the gains come In");
	addPort("gainsOut",portGansOut).doc("Where the gains come Out");

	addPort("stateEstimate",portStateEstimate).doc("The estimated state of the system");

	memset(&lineAngles, 0, sizeof( lineAngles ));
	memset(&gains, 0, sizeof( gains ));
	memset(&reference, 0, sizeof( reference ));
	memset(&stateEstimate, 0, sizeof( stateEstimate ));

	memset(&debug, 0, sizeof( debug ));

	trigger = TimeService::Instance()->getTicks();
	trigger_last = TimeService::Instance()->getTicks();
	trigger_last_is_valid = false;

}

bool KalmanFilter::configureHook()
{
	FlowStatus gainsStatus = portGainsIn.read(gains);
	if(gainsStatus != NewData) 
	{
		log(Error) << "kalmanFilter: Cannot configure; gains are needed!" << endlog();
		return false;
	}

	FlowStatus referenceStatus = portReference.read(reference);
	if(referenceStatus != NewData) 
	{
		log(Error) << "kalmanFilter: Cannot configure; reference state is needed!" << endlog();
		return false;
	}

	// Load in the highReference and lowReference


	return true;
}

bool  KalmanFilter::startHook()
{
	return true;
}

void  KalmanFilter::updateHook()
{

	TIME_TYPE trigger = TimeService::Instance()->getTicks();
	//portDriveState.read(driveState);

	//resampledMeasurements.ts_trigger = trigger;
	//resampledMeasurements.ts_elapsed = TimeService::Instance()->secondsSince( trigger );

	//Matrix<double,STATECOUNT,1> resampledMeasurements;

	//portData.write(resampledMeasurements);

}

void  KalmanFilter::stopHook()
{}

void  KalmanFilter::cleanupHook()
{}

void  KalmanFilter::errorHook()
{}

ORO_CREATE_COMPONENT( KalmanFilter )
