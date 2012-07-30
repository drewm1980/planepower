#ifndef __LEDTRACKERSIMULATOR__
#define __LEDTRACKERSIMULATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <ocl/OCL.hpp>

#include <fstream>
#include <unistd.h>
#include <stdint.h>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

#define TIME_TYPE uint64_t

namespace OCL
{

	/// LEDTrackerSimulator class
	/**
	This class simulates the free motion of a ball attached to a pendulum.
	The pendulum motion is executed in the x=0 plane of the pendulum reference
	frame. The state of the ball in the pendulum plane is given by
	[theta,omega,alpha]. The position of the ball in the world frame is given by
	[x,y,z].
	The pendulum reference frame wrt to the world reference frame gives the pose
	of the pendulum motion plane wrt to the world.
	*/
	class LEDTrackerSimulator
		: public TaskContext
	{
	protected:
		/*********
		PROPERTIES
		*********/
		//! Example of a property
		Property<double>			_prop;

		/*********
		DATAPORTS
		*********/
		//! Input port
		InputPort<vector<double> >		_markerPositionsIn;
		InputPort<TIME_TYPE>			_triggerTimeStampIn;
		OutputPort<TIME_TYPE>			_triggerTimeStampOut;
		TIME_TYPE				triggerTimeStamp;
		InputPort<vector<double> >		_markerPositionsAndCovarianceIn;
		OutputPort<vector<double> >		_markerPositionsOut;
		OutputPort<vector<double> >		_markerPositionsAndCovarianceOut;

	private:
		vector<double>				markerPositions;
		vector<double>				markerPositionsAndCovariance;
		double					computation_time;

	public:
		LEDTrackerSimulator(std::string name);
		~LEDTrackerSimulator();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
		
	};
}
#endif // __LEDTRACKERSIMULATOR__
