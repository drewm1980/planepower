#include "LEDTracker.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <cmath>

using namespace std;
using namespace RTT;
using namespace RTT::os;

#include "types.hpp"
#include "cout.hpp"

#include "pose_from_markers.h"

#define VERBOSE 0

#define MARKER_NOT_DETECTED_VALUE -1.0

LEDTracker::LEDTracker(std::string name)
	: TaskContext(name, PreOperational)
{
	properties()->addProperty("useExternalTrigger", _useExternalTrigger)
		.doc("Set to true if the camera is triggered externally, otherwise LEDTracker will set the camera to free-running mode.");
	// Flag for external triggering. By default it is false.
	_useExternalTrigger =false;

	addPort("data", portData)
		.doc("Output data port of the component.");

	addPort("triggerTimeStampIn", _triggerTimeStampIn)
		.doc("Input time stamp.");

	properties()->addProperty("sigma_marker", sigma_marker)
		.doc("The standard deviation of the camera measurements. Default = 1e3");
	sigma_marker = 1e3;
	
	data.positions.resize(CAMERA_COUNT * LED_COUNT * 2, 0.0);
	data.weights.resize(CAMERA_COUNT * LED_COUNT * 2, 0.0);
	data.pose.resize(NPOSE, 0.0);
	data.ts_trigger = TimeService::Instance()->getTicks();
	portData.setDataSample( data );
	portData.write( data );
}

LEDTracker::~LEDTracker()
{}

bool  LEDTracker::configureHook()
{
	Logger::In in( getName() );

	cameraArray = new CameraArray( _useExternalTrigger );
	if (cameraArray->initialized() == false)
	{
		log( Error ) << "Camera array failed to initialize itself." << endlog();
		return false;
	}

	frame_w = cameraArray->frame_w;
	frame_h = cameraArray->frame_h;
	for(int i=0; i < CAMERA_COUNT; i++)
	{
		blobExtractors[i] = new BlobExtractor(frame_w, frame_h, NEED_TO_DEBAYER);
	}
	return true;
}

bool  LEDTracker::startHook()
{
	if (cameraArray->initialized() == false)
		return false;

	cameraArray->startHook();
	return true;
}

void  LEDTracker::updateHook()
{
	TIME_TYPE triggerTimeStamp, frameArrivalTimeStamp;

	// This blocks until a frame arrives from all cameras
	cameraArray->updateHook();

	// The timestamp the camera was triggered for the current frame.
	while(_triggerTimeStampIn.read( triggerTimeStamp ) == NewData); 
//	_triggerTimeStampIn.read(triggerTimeStamp);
	
	// Read the camera frame arrival time!
	frameArrivalTimeStamp = TimeService::Instance()->getTicks();

#if VERBOSE
	double transferTime = (frameArrivalTimeStamp-triggerTimeStamp)*1e-9; // sec
	COUT << "Transfer time was: " << transferTime*1e3 << "ms" << ENDL;
#endif

	// Note, this ~could trivially be done in parallel!
	for(unsigned i = 0; i < CAMERA_COUNT; i++)
		blobExtractors[ i ]->find_leds( cameraArray->current_frame_data[ i ] );
	
	// Copy marker location data from the extractors into our staging area.
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		MarkerLocations * src = &(blobExtractors[i]->markerLocations);	
		MarkerLocations * dst = ((MarkerLocations*) &(data.positions[ 0 ])) + i;
		*dst = *src;
	}

	// Rescale the pixel data.
	// Note!!!! whatever you do, make sure this is appropriate for the
	// camera calibration files you're using, or your camera measurement
	// function will be broken!!!
	if(cameraArray->frame_w == 800)
	{
		for(unsigned int i=0; i<CAMERA_COUNT*LED_COUNT*2; i++)
		{
			if( !isnan( data.positions[ i ] ) )
			{ 
				// Match old camera resolution until we redo geometric calibration
				data.positions[ i ] *= 2; 
			} 
		}
	}
	
	// If a Marker was not detected properly,
	// put an arbitrary value and set the weight to 0
	// otherwise, set the weight properly
	bool foundNaN = false;
	for(unsigned i = 0; i < CAMERA_COUNT * LED_COUNT * 2; i++)
		if( isnan( data.positions[ i ] ) )
		{
			data.positions[ i ] = MARKER_NOT_DETECTED_VALUE;
			data.weights[ i ]   = 0.0;
			foundNaN = true;
		}
		else
		{
			data.weights[ i ]   = 1.0 / (sigma_marker * sigma_marker);
		}

	//
	// Calculate pose form marker positions
	//
	poseFromMarkers( foundNaN );
	
	//
	// Fill in the timings and send data to the output port
	//
	data.ts_trigger = triggerTimeStamp; 
	data.ts_frame   = frameArrivalTimeStamp;
	data.ts_elapsed = TimeService::Instance()->secondsSince( triggerTimeStamp );

	portData.write( data );

	// Tell orocos to re-trigger this component immediately after
	// it is done updating output ports, so that it can start waiting
	// for the next frame arrival
	this->getActivity()->trigger();
}

void  LEDTracker::stopHook()
{
	for (unsigned i = 0; i < CAMERA_COUNT * LED_COUNT * 2; i++)
	{
		data.positions[ i ] = MARKER_NOT_DETECTED_VALUE;
		data.weights[ i ]   = 0.0;
	}
	for (unsigned i = 0; i < NPOSE; ++i)
		data.pose[ i ] = 0.0;

	portData.write( data );

	cameraArray->stopHook();
}

void  LEDTracker::cleanupHook()
{
	cameraArray->cleanUpHook();
	delete cameraArray;
	for(int i=0; i<CAMERA_COUNT; i++)
	{
		delete blobExtractors[i];
	}
}

void LEDTracker::errorHook()
{}

void LEDTracker::poseFromMarkers(bool foundNaN)
{
	double* cInput[ 1 ] = { cMarkers };
	double* cOutput[ 1 ] = { cPose };
	int cStatus = 1;

	if (foundNaN == false)
	{
		copy(data.positions.begin(), data.positions.end(), cInput[ 0 ]);

		cStatus = pose_from_markers(cInput, cOutput);
	}
	
	if (cStatus != 0 || foundNaN == true)
	{
		memset(cPose, 0.0, NPOSE * sizeof( double ));
	}
	
	// Assign data to output buffer
	data.pose.assign(cPose, cPose + NPOSE);
}

ORO_CREATE_COMPONENT( LEDTracker )
