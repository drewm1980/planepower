#include "LEDTracker.hpp"
#include <ocl/Component.hpp>
#include <rtt/marsh/Marshalling.hpp>

#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include "types.hpp"
//#include "blob_extractors.hpp"
#include "cout.hpp"

#define VERBOSE 0

ORO_CREATE_COMPONENT( OCL::LEDTracker)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL {

LEDTracker::LEDTracker(std::string name) : TaskContext(name)
{
	attributes()->addAttribute( "useExternalTrigger", _useExternalTrigger);
	_useExternalTrigger.set(false); // Default Value

	addPort( "markerPositions",_markerPositions ).doc("Pixel Locations of the markers");
	addPort( "markerPositionsAndCovariance",_markerPositionsAndCovariance ).doc("Pixel locatoins and weights");
	addPort("triggerTimeStampIn",_triggerTimeStampIn);
	addPort("triggerTimeStampOut",_triggerTimeStampOut);

	addPort("compTime",_compTime);

	addPort("frameArrivalTimeStamp",_frameArrivalTimeStamp);
	addPort("computationCompleteTimeStamp",_computationCompleteTimeStamp);

	addPort("deltaIn",_deltaIn);
	addPort("deltaOut",_deltaOut);

	markerPositions.resize(12,0.0);
	markerPositionsAndCovariance.resize(24,0.0);

	addProperty( "sigma_marker",sigma_marker).doc("The standard deviation of the camera measurements. Default = 1e3");
	sigma_marker = 1e3;

	tempTime = RTT::os::TimeService::Instance()->getTicks(); // Get current time

	_markerPositions.setDataSample( markerPositions );
	_markerPositions.write( markerPositions );
	_markerPositionsAndCovariance.setDataSample( markerPositionsAndCovariance );
	_markerPositionsAndCovariance.write( markerPositionsAndCovariance );
	_triggerTimeStampOut.setDataSample( tempTime );
	_triggerTimeStampOut.write( tempTime );
	_compTime.setDataSample( tempTime );
	_compTime.write( tempTime );
	_frameArrivalTimeStamp.setDataSample( tempTime );
	_frameArrivalTimeStamp.write( tempTime );
	_computationCompleteTimeStamp.setDataSample( tempTime );
	_computationCompleteTimeStamp.write( tempTime );
	_deltaOut.setDataSample( 0.0 );
	_deltaOut.write( 0.0 );
}

LEDTracker::~LEDTracker()
{
}

bool  LEDTracker::configureHook()
{
	stereoPair = new StereoPair(_useExternalTrigger.get());
	blobExtractor1 = new BlobExtractor(stereoPair->frame_w, stereoPair->frame_h);
	blobExtractor2 = new BlobExtractor(stereoPair->frame_w, stereoPair->frame_h);
	return true;
}

bool  LEDTracker::startHook()
{
	stereoPair->startHook();
	return true;
}

void  LEDTracker::updateHook()
{
	// This blocks until a frame arrives
	stereoPair->updateHook();
	_triggerTimeStampIn.read(triggerTimeStamp); // The timestamp the camera was triggered for the current frame.

	_deltaIn.read(delta);

	frameArrivalTimeStamp = RTT::os::TimeService::Instance()->getTicks(); // Get current time 
	_frameArrivalTimeStamp.write(frameArrivalTimeStamp);
#if VERBOSE
	double transferTime = (frameArrivalTimeStamp-triggerTimeStamp)*1e-9; // sec
	COUT << "Transfer time was: " << transferTime*1e3 << "ms" << ENDL;
#endif

	// This blocks until computation is complete
	tempTime = RTT::os::TimeService::Instance()->getTicks(); // Refresh timestamp, in case PRINTF took time.
	blobExtractor1->extract_blobs(stereoPair->right_frame_data);
	blobExtractor2->extract_blobs(stereoPair->left_frame_data);
	computationCompleteTimeStamp = RTT::os::TimeService::Instance()->getTicks(); // Get current time 
	//double computationTime = (computationCompleteTimeStamp - tempTime)*1e-9; // sec
	double computationTime = (computationCompleteTimeStamp - triggerTimeStamp)*1e-9; // sec
#if VERBOSE
	//COUT << "Total computation time was: " << computationTime*1.0e3/trials << "ms" << ENDL;
#endif
	_compTime.write(computationTime*1.0e3);
	
#if 0 //VERBOSE
	COUT 
		<< blobExtractor1->markerLocations.red.x << ' '
		<< blobExtractor1->markerLocations.red.y << ' '
		<< blobExtractor1->markerLocations.green.x << ' '
		<< blobExtractor1->markerLocations.green.y << ' '
		<< blobExtractor1->markerLocations.blue.x << ' '
		<< blobExtractor1->markerLocations.blue.y << ' ' 
		<< blobExtractor2->markerLocations.red.x << ' '
		<< blobExtractor2->markerLocations.red.y << ' '
		<< blobExtractor2->markerLocations.green.x << ' '
		<< blobExtractor2->markerLocations.green.y << ' '
		<< blobExtractor2->markerLocations.blue.x << ' '
		<< blobExtractor2->markerLocations.blue.y << ' '
	<< ENDL;
#endif

	markerPositions[0] = blobExtractor1->markerLocations.red.x;
	markerPositions[1] = blobExtractor1->markerLocations.red.y;
	markerPositions[2] = blobExtractor1->markerLocations.green.x;
	markerPositions[3] = blobExtractor1->markerLocations.green.y;
	markerPositions[4] = blobExtractor1->markerLocations.blue.x;
	markerPositions[5] = blobExtractor1->markerLocations.blue.y;
	markerPositions[6] = blobExtractor2->markerLocations.red.x;
	markerPositions[7] = blobExtractor2->markerLocations.red.y;
	markerPositions[8] = blobExtractor2->markerLocations.green.x;
	markerPositions[9] = blobExtractor2->markerLocations.green.y;
	markerPositions[10] = blobExtractor2->markerLocations.blue.x;
	markerPositions[11] = blobExtractor2->markerLocations.blue.y;
	for(unsigned int i=0; i<12; i++)
	{
		if(isnan(markerPositions[i])){ // Marker was not detected properly.. Put a random value and puth the weight to 0
			markerPositionsAndCovariance[i] = -1000.0;
			markerPositionsAndCovariance[i+12] = 0.0;
		}
		else{
		markerPositionsAndCovariance[i] = markerPositions[i];
		markerPositionsAndCovariance[i+12] = 1.0/(sigma_marker*sigma_marker);
		}
	}

	//markerPositions[0] = 250;
	//markerPositions[1] = 675;
	//markerPositions[2] = 613;
	//markerPositions[3] = 810;
	//markerPositions[4] = 468;
	//markerPositions[5] = 420;
	//markerPositions[6] = 253;
	//markerPositions[7] = 633;
	//markerPositions[8] = 609;
	//markerPositions[9] = 782;
	//markerPositions[10] = 413;
	//markerPositions[11] = 343;

	_triggerTimeStampOut.write(triggerTimeStamp);
	_markerPositions.write(markerPositions);
	_markerPositionsAndCovariance.write(markerPositionsAndCovariance);

	_deltaOut.write(delta);
	// Tell orocos to re-trigger this component immediately after
	// it is done updating output ports
	this->getActivity()->trigger();
}

void  LEDTracker::stopHook()
{
	stereoPair->stopHook();
}

void  LEDTracker::cleanUpHook()
{
	stereoPair->cleanUpHook();
	delete stereoPair;
	delete blobExtractor1;
	delete blobExtractor2;
}

}//namespace

