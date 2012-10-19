#ifndef __PLANESIMULATORRFRAME__
#define __PLANESIMULATORRFRAME__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <stdlib.h>
#include <ocl/OCL.hpp>

#include <fstream>
#include <stdint.h>
#include <stdlib.h>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

#define NSTATES 22
#define NIMU 6
#define NCONTROLS 3
#define SCALE_UR	1.25e6	// Scaling of ur_1, ur_2
#define SCALE_UP	2e5		// Scaling of u_p
typedef uint64_t TIME_TYPE;

namespace OCL
{

    /// planeSimulatorRframe class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class planeSimulatorRframe
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
	InputPort<TIME_TYPE>				_trigger;
        InputPort<vector<double> >			_controlInputPort;
        //! Output port
        OutputPort<vector<double> >			_stateOutputPort;
	OutputPort<TIME_TYPE>				_triggerTimeStampOut;
	TIME_TYPE					triggerTimeStamp;
        OutputPort<vector<double> >			_IMUOutputPort;
	OutputPort<vector<double> >			_deltaOmega;
	vector<double>					deltaOmega;
	OutputPort<vector<double> >			_controlOutputPort;
	vector<double>					controlOutputPort;

    private:
        int					initialiseState(vector<double>);
	double					h; //The stepsize of the integrator (the period of the component)
	vector<double>				X; //The state of the system, augmented with the accelerations
	vector<double>				X1; //The state of the system, augmented with the accelerations
	vector<double>				IMU;
	vector<double>				U; //The state of the system


	int rk4Wrapper(vector<double> X, vector<double> U, double h, vector<double> *X1, vector<double> *IMU);
	
	double					rk4Wrapper_input_X[NSTATES];
	double					rk4Wrapper_input_U[NCONTROLS];
	double					rk4Wrapper_input_h[1];
	double*					rk4Wrapper_input[3];

	double					rk4Wrapper_output_X1[NSTATES];
	double					rk4Wrapper_output_IMU[NIMU];
	double*					rk4Wrapper_output[2];

	RTT::os::TimeService::ticks     time_begin;

    public:
        planeSimulatorRframe(std::string name);
        ~planeSimulatorRframe();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
        
    };
}
#endif // __SIMULATOR__
