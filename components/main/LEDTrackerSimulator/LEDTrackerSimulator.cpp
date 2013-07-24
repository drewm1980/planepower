#include "LEDTrackerSimulator.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::LEDTrackerSimulator)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 LEDTrackerSimulator::LEDTrackerSimulator(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		addProperty( "computation_time",computation_time).doc("The computation time for extracting the LEDs to use for simulation [sec]");
		computation_time = 75.0*1e-3;
		// Add ports

		addEventPort( "triggerTimeStampIn",_triggerTimeStampIn );
		addPort( "triggerTimeStampOut",_triggerTimeStampOut );
		addPort( "markerPositionsIn",_markerPositionsIn ).doc("markerPositionsIn");
		addPort( "markerPositionsAndCovarianceIn",_markerPositionsAndCovarianceIn ).doc("markerPositionsAndCovarianceIn");
		addPort( "markerPositions",_markerPositionsOut ).doc("markerPositions");
		addPort( "markerPositionsAndCovariance",_markerPositionsAndCovarianceOut ).doc("markerPositionsAndCovariance");

	}
	

	LEDTrackerSimulator::~LEDTrackerSimulator()
	{
	}

	bool  LEDTrackerSimulator::configureHook()
	{
		return true;
	 }

	bool  LEDTrackerSimulator::startHook()
	{
		return true;
	}

	void  LEDTrackerSimulator::updateHook()
	{
		_triggerTimeStampIn.read(triggerTimeStamp);
		_markerPositionsIn.read(markerPositions);
		_markerPositionsAndCovarianceIn.read(markerPositionsAndCovariance);
		usleep(computation_time*1e6);
		_markerPositionsOut.write(markerPositions);
		_markerPositionsAndCovarianceOut.write(markerPositionsAndCovariance);
		_triggerTimeStampOut.write(triggerTimeStamp);
	}

	void  LEDTrackerSimulator::stopHook()
	{
	}

	void  LEDTrackerSimulator::cleanUpHook()
	{
	}

}//namespace

