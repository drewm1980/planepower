#ifndef __CONTROLSPLAYBACK__
#define __CONTROLSPLAYBACK__

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
#include <math.h>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

#define SEQUENCE_FILENAME STUPID_LONG_PATH "sequence.dat"
typedef uint64_t TIME_TYPE;

namespace OCL
{

	/// ControlsPlayback class
	/**
	This class simulates the free motion of a ball attached to a pendulum.
	The pendulum motion is executed in the x=0 plane of the pendulum reference
	frame. The state of the ball in the pendulum plane is given by
	[theta,omega,alpha]. The position of the ball in the world frame is given by
	[x,y,z].
	The pendulum reference frame wrt to the world reference frame gives the pose
	of the pendulum motion plane wrt to the world.
	*/
	class ControlsPlayback
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
		InputPort<TIME_TYPE>			_triggerTimeStampIn;
		//! Output port
		OutputPort<vector<double> >		_controlOutputPort;
		vector<double>				controlOutput;

	private:
		void loadMatrixFromDat(const char *filename, vector<vector<double> > &V);
		bool startSequence();
		bool stopSequence();
		bool loadSequence();
		vector<vector<double> >			Sequence;
		bool					sequencing;
		unsigned int				i_sequence;
		unsigned int				N_sequence;

	public:
		ControlsPlayback(std::string name);
		~ControlsPlayback();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
		
	};
}
#endif // __CONTROLSPLAYBACK__
