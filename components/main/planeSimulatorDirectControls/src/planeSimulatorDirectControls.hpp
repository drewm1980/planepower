#ifndef __planeSimulatorDirectControls__
#define __planeSimulatorDirectControls__

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

namespace OCL
{

    /// planeSimulatorDirectControls class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class planeSimulatorDirectControls
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
        InputPort<vector<double> >			_controlInputPort;
        //! Output port
        OutputPort<vector<double> >			_stateOutputPort;

    private:
        int					initialiseState(vector<double>);
	double					h; //The stepsize of the integrator (the period of the component)
	vector<double>				X_reduced; //The reduced state of the system, augmented with the accelerations
	vector<double>				X; //The state of the system, augmented with the accelerations
	vector<double>				U; //The controls of the system
	vector<double>				U_direct; //The direct controls of the system
	double					X_array[24]; //The state as an array
	double					U_array[4]; //The control as an array
	double					h_array[1];
	double*					full_array[3]; //array containing state, control and time step
	double					X1_array[27]; //The next state augmented with the accelerations as an array
	double*					X1_big_array[1]; //The next state augmented with the accelerations as an array

	RTT::os::TimeService::ticks     time_begin;

    public:
        planeSimulatorDirectControls(std::string name);
        ~planeSimulatorDirectControls();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
        
    };
}
#endif // __SIMULATOR__
