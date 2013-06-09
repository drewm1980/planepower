#include "masterTimer.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

MasterTimer::MasterTimer(string name)
	: TaskContext(name, PreOperational)
{
	//
	// Add properties
	//
	addProperty( "imu_target_hz",_imu_target_hz )
		.doc("The target measurement rate of the imu (Hz)");
	addProperty( "camera_target_hz",_camera_target_hz )
		.doc("The target framerate of the camera (Hz)");
	addProperty( "controls_playback_target_hz",_controls_playback_target_hz )
		.doc("The target framerate of the camera (Hz)");

	//
	// Add ports
	//
	addPort( "cameraClock",_cameraClock )
		.doc("The description of the port");
	addPort( "imuClock",_imuClock )
		.doc("The description of the port");
	addPort( "controlsPlaybackClock",_controlsPlaybackClock )
		.doc("The description of the port");
	addPort("imuCameraRatio" , _imuCameraRatio)
		.doc("The ratio of imu freq. on camera freq.");

	addPort("deltaIn",_deltaIn);
	addPort("deltaOut",_deltaOut);

	// Get current time
	myticks = TimeService::Instance()->getTicks();

	_imuClock.setDataSample( myticks );
	_imuClock.write( myticks );
	_cameraClock.setDataSample( myticks );
	_cameraClock.write( myticks );
	_controlsPlaybackClock.setDataSample( myticks );
	_controlsPlaybackClock.write( myticks );
	_masterClock.setDataSample( myticks );
	_masterClock.write( myticks );
	_imuCameraRatio.setDataSample( 0 );
	_imuCameraRatio.write( 0 );
	_deltaOut.setDataSample( 0 );
	_deltaOut.write( 0 );

	base_clock_index = 0;
}


bool MasterTimer::configureHook()
{
	if (this->getActivity()->isPeriodic() == false)
	{
		log( Error ) << "This component must be periodic." << endlog();
		return false;
	}

	double myperiod = this->getActivity()->getPeriod();

	portPointers[0] = &_imuClock;
	portPointers[1] = &_cameraClock;
	portPointers[2] = &_controlsPlaybackClock;
	for(int i=0; i<CLOCK_COUNT; i++)
	{
		portPointers[i]->setDataSample(myticks);
		portPointers[i]->write(myticks);
	}

	for(int i=0; i<CLOCK_COUNT; i++)
	{
		double target_period = 1.0 / target_hz[i];

		double temp_divider = target_period/myperiod;
		double thresh = fabs((temp_divider - round(temp_divider))/temp_divider);
		if( thresh < 1.0e-6)
		{
			// The divider was probably intended to be the integer.
			dividers[i] = round(temp_divider);
		} else {
			// The divider needs to be rounded up.
			dividers[i] = ceil(temp_divider);
		}
		log( Info ) << "Master timer divider " << i << " = " 
					<< dividers[ i ] << endlog();
	}
	// Write the ratio of IMU measurements on camera measurements on a port
	_imuCameraRatio.write(dividers[ 1 ] / dividers[ 0 ]);
	return true;
}

bool  MasterTimer::startHook()
{
	return true;
}

void  MasterTimer::updateHook()
{
	// Get current time
	myticks = TimeService::Instance()->getTicks();

	for (int i = 0; i < CLOCK_COUNT; i++)
	{
		if (base_clock_index % dividers[ i ]==0)
		{
			// Trigger event port i
			if (i == 1)
			{
				// Camera clock
				_deltaIn.read(delta);
				_deltaOut.write(delta);
			}
			portPointers[i]->write(myticks);
		}
	}
	_imuCameraRatio.write(dividers[ 1 ] / dividers[ 0 ]);
	base_clock_index++;
}

void  MasterTimer::stopHook()
{}

void  MasterTimer::cleanupHook()
{}

void  MasterTimer::errorHook()
{}

ORO_CREATE_COMPONENT( MasterTimer )
