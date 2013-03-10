#ifndef __CAMERASIMULATOR__
#define __CAMERASIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <stdint.h>

#include <ocl/OCL.hpp>

#include <fstream>
#include <stdlib.h>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

#define TIME_TYPE uint64_t

namespace OCL
{

    /// CameraSimulator class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class CameraSimulator
        : public TaskContext
    {
    protected:
        InputPort<vector<double> >		_stateInputPort;
        InputPort<TIME_TYPE>			_triggerTimeStampIn;
		OutputPort<TIME_TYPE>			_triggerTimeStampOut;
		TIME_TYPE						triggerTimeStamp;
		OutputPort<vector<double> >			_markerPositions;
		OutputPort<vector<double> >			_markerPositionsAndCovariance;

		OutputPort<TIME_TYPE>			_frameArrivalTimeStamp; // downstream components should event trigger on this
		TIME_TYPE						frameArrivalTimeStamp;

		// The following ports, variables are NOT actually used;
		// they are only here for interface compatibility.
		Attribute<bool>					_useExternalTrigger; 
		//OutputPort<TIME_TYPE>			_compTime; 
		//OutputPort<TIME_TYPE>			_computationCompleteTimeStamp; 
		//TIME_TYPE						computationCompleteTimeStamp;
		//InputPort<double>				_deltaIn; 
		//OutputPort<double>				_deltaOut; 
		//double delta;

    private:
		// The full state passed by the plane simulator:
		// x,y,z,dx,dy,dz,e11,e12,e13,e21,e22,e23,e31,e32,e33,w1,w2,w3,delta,ddelta,ua,ue
		//  (in tether attachment point coordinates)
#define NFULLSTATE (3+3+9+3+1+1+1+1)
		vector<double>  fullstate;	

		// The static part of the state, in "Mario" order:
		// x,y,z,e11,e12,e13,e21,e22,e23,e31,e32,e33 , in tether attachment point coordinates
#define NPOSE (3+9)
		vector<double>  pose;	

#define NMARKERS (12)
		vector<double>	markerPositions;
		vector<double>  markerPositionsAndCovariance; // Marker positions followed by the diagonal of the covariance matrix of the marker positions.
		double			sigma_marker;
		double			transfer_time; // Time for simulated transfer between pose arrival and marker positions(standing in for camera image) output. // in sec

		void getMarkersWrapper(vector<double> pose, vector<double> *markerPositions);

    public:
        CameraSimulator(std::string name);
        ~CameraSimulator();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
        
    };
}
#endif // __CAMERASIMULATOR__
