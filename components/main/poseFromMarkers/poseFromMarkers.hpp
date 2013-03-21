#ifndef __POSEFROMMARKERS__
#define __POSEFROMMARKERS__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <ocl/OCL.hpp>

#include <fstream>

// #include "casadi_functions.cpp"

#define NMARKERPOSITIONS 12
#define NPOSE 12

using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

namespace OCL
{

    /// PoseFromMarkers class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class PoseFromMarkers
        : public TaskContext
    {
    protected:
        /*********
        DATAPORTS
        *********/
        //! Input port
        InputPort<vector<double> >			_markerPositions;
	vector<double>					markerPositions;
        //! Output port
        OutputPort<vector<double> >			_pfm;
        //OutputPort<vector<double> >			_cov_rt;
        //OutputPort<vector<double> >			_cov_rt_inv;
        OutputPort<vector<double> >			_pose_and_cov_rt_inv;
	vector<double>					pfm;
	vector<double>					cov_rt;
	vector<double>					cov_rt_inv;
	vector<double>					pose_and_cov_rt_inv;

    private:
        void					getPose();

	int					getPoseWrapper(vector<double> markerPositions
									, vector<double> *pose
									, vector<double> *cov_rt
									, vector<double> *cov_rt_inv);

	double					getPoseWrapper_markerPositions[NMARKERPOSITIONS];
	double*					getPoseWrapper_input[1];

	double					getPoseWrapper_pose[NPOSE];
	double					getPoseWrapper_cov_rt[NPOSE*NPOSE];
	double					getPoseWrapper_cov_rt_inv[NPOSE*NPOSE];
	double*					getPoseWrapper_output[3];

    public:
        PoseFromMarkers(std::string name);
        ~PoseFromMarkers();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
        
    };
}
#endif // __POSEFROMMARKERS__
