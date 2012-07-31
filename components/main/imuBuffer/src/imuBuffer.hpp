#ifndef __IMUBUFFER__
#define __IMUBUFFER__

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
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;
#include <stdint.h>

typedef uint64_t TIME_TYPE;

namespace OCL
{

    /// ImuBuffer class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class ImuBuffer
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
        InputPort<vector<double> >		_imuData;
	vector<double>				imuData;
	InputPort<int>				_imuCameraRatio;
        //! Output port
        OutputPort<vector<double> >		_imuMeanCov;
        vector<double>				imuMeanCov;

    private:
	void					addMeasurement();
	bool					first;

	vector<vector<double> >			halfBuffer;		// Buffer of data of the last measurements that span half a camera period
	unsigned int				halfBufferIndex;
	vector<vector<double> >			fullBuffer;		// Buffer of data of a full period that starts after half a period of new data
	unsigned int				fullBufferIndex;

	vector<double>				imuMean_last;		// The mean of the last samples
	vector<double>				imucov_last;		// The covariance of the last samples
	vector<double>				imuMean_prev;		// The mean of the second to last samples
	vector<double>				imucov_prev;		// The covariance of the second to last samples

	double acc_scale;
	double angvel_scale;

    public:
        ImuBuffer(std::string name);
        ~ImuBuffer();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
        
    };
}
#endif // __IMUBUFFER__
