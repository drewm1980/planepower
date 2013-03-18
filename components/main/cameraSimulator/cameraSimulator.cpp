#include "cameraSimulator.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::CameraSimulator)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

#include "markers_from_pose.h"

namespace OCL
{
     CameraSimulator::CameraSimulator(std::string name)
         : TaskContext(name)
     {
        log(Debug) << "(CameraSimulator) constructor entered" << endlog();

		addEventPort("triggerTimeStampIn",_triggerTimeStampIn).doc("Timestamp associated with the input state");
		addPort("stateInputPort",_stateInputPort).doc("State, usually from plane simulator");
		addPort("triggerTimeStampOut",_triggerTimeStampOut).doc("Timestamp associated with the output markers");
		addPort( "markerPositions",_markerPositions ).doc("Pixel Locations of the markers");
		addPort( "markerPositionsAndCovariance",_markerPositionsAndCovariance ).doc("Pixel locatoins and weights");

		//addPort("compTime",_compTime).doc("Computation Time for the whole camera+BlobExtractor setup");

		addPort("frameArrivalTimeStamp",_frameArrivalTimeStamp);
		//addPort("computationCompleteTimeStamp",_computationCompleteTimeStamp);

		//addPort("deltaIn",_deltaIn).doc("Carousel angle at time frame was taken, for debugging");
		//addPort("deltaOut",_deltaOut).doc("Carousel angle at time frame was taken, for debugging");

		fullstate.resize(NFULLSTATE);
		pose.resize(NPOSE);
		markerPositions.resize(NMARKERS);
		markerPositionsAndCovariance.resize(NMARKERS+NMARKERS); // Marker positions followed by the diagonal of the covariance matrix of the marker positions.

		addProperty( "sigma_marker",sigma_marker).doc("The standard deviation of the camera measurements [pixels]");
		addProperty( "transfer_time",transfer_time).doc("The camera data transfer time to use for simulation [sec]");
		sigma_marker = 1000.0;
		transfer_time = 75.0*1e-3; // sec

        log(Debug) << "(CameraSimulator) constructor finished" << endlog();
    }

    CameraSimulator::~CameraSimulator()
    {
    }

    bool  CameraSimulator::configureHook()
    {
        log(Debug) << "(CameraSimulator) ConfigureHook entered" << endlog();
        log(Debug) << "(CameraSimulator) ConfigureHook finished" << endlog();
        
        return true;
     }

    bool  CameraSimulator::startHook()
    {
        log(Debug) << "(CameraSimulator) startHook entered" << endlog();
        log(Debug) << "(CameraSimulator) startHook finished" << endlog();
        return true;
    }

    void  CameraSimulator::updateHook()
    {
        //log(Debug) << "(CameraSimulator) updateHook entered" << endlog();

		_triggerTimeStampIn.read(triggerTimeStamp);
		_stateInputPort.read(fullstate);
		
		// Copy static part of state into pose
		copy(fullstate.begin(), fullstate.begin()+3, pose.begin());
		copy(fullstate.begin()+6, fullstate.begin()+6+9, pose.begin()+3);

		getMarkersWrapper(pose,&markerPositions);
		for(unsigned int i=0; i<markerPositions.size(); i++){
			markerPositions[i] += (drand48()-0.5)*20.0; // Add noise
		}
		_markerPositions.write(markerPositions);
		
		//double invcov = 1.0/(sigma_marker*sigma_marker);
		double invcov = 1e-6;

		// Fill in our output for markers and covariance
		copy(markerPositions.begin(),markerPositions.end(), markerPositionsAndCovariance.begin());
		for(int i=0; i<NMARKERS; i++)
		{
			markerPositionsAndCovariance[NMARKERS+i] = invcov;
		}

		usleep(transfer_time*1.0e6);
		TIME_TYPE frameArrivalTimeStamp = os::TimeService::Instance()->getTicks();

		_markerPositions.write(markerPositions);
		_markerPositionsAndCovariance.write(markerPositionsAndCovariance);

		_triggerTimeStampOut.write(triggerTimeStamp);
		_frameArrivalTimeStamp.write(frameArrivalTimeStamp); // downstream components should event trigger on this
	}

	void CameraSimulator::getMarkersWrapper(vector<double> pose, vector<double> *markerPositions)
	{
		// Casadi's functions require vectors of pointers to inputs.
		double* inputs[1] = {&(pose[0])};
		double* outputs[1];
		double* temp = &((*markerPositions)[0]);
		outputs[0] = temp;
		// Call wrapped function
		markers_from_pose(inputs, outputs);
	}

    void  CameraSimulator::stopHook()
    {
        log(Debug) << "(CameraSimulator) stopHook entered" << endlog();
        log(Debug) << "(CameraSimulator) stopHook finished" << endlog();
    }

    void  CameraSimulator::cleanUpHook()
    {
        log(Debug) << "(CameraSimulator) cleanupHook entered" << endlog();
        log(Debug) << "(CameraSimulator) cleanupHook finished" << endlog();
    }

}//namespace

