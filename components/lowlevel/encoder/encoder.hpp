#ifndef __ENCODER__
#define __ENCODER__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <rtt/os/TimeService.hpp>

#include <ocl/OCL.hpp>

#include <math.h>
#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;
#include <stdint.h>
typedef uint64_t TIME_TYPE;

#define REGSIZE 10

namespace OCL
{

    /// Encoder class
    /**
    This class simulates the free motion of a ball attached to a pendulum.
    The pendulum motion is executed in the x=0 plane of the pendulum reference
    frame. The state of the ball in the pendulum plane is given by
    [theta,omega,alpha]. The position of the ball in the world frame is given by
    [x,y,z].
    The pendulum reference frame wrt to the world reference frame gives the pose
    of the pendulum motion plane wrt to the world.
    */
    class Encoder
        : public TaskContext
    {
    protected:
        /*********
        PROPERTIES
        *********/

        /*********
        DATAPORTS
        *********/
        //! Output port
        OutputPort<double>			_delta;
        OutputPort<double>			_omega;
	InputPort<TIME_TYPE>			_encoderTimeStampIn;
	OutputPort<TIME_TYPE>			_encoderTimeStampOut;
	TIME_TYPE				encoderTimeStamp;
	OutputPort<TIME_TYPE>			_computationTime;
	OutputPort<vector<double> >		_deltaOmega;
	vector<double>				deltaOmega;

        /*********
        OPERATIONS
        *********/
	OperationCaller<int(unsigned int) >	readEncoder;


    private:
	RTT::os::TimeService::ticks		tickRegister; // Previous time of updateHook
	RTT::os::TimeService::ticks		myticks; // Current time of updateHook
	RTT::os::TimeService::nsecs		deltansec; // Time difference between current and previous updateHook in nanoseconds
	double					myperiod; // Time difference between current and previous updateHook in seconds
	int					mycount_new; // Current ticks of encoder
	int					mycount_old; // Previous ticks of encoder
	unsigned int				encoderPort; // The port the encoder is hooked up to
	int					diff;

	int					regi;
	int					diffcount[REGSIZE];  //regsize
	double					difftime[REGSIZE];   //regsize	
	double 					omega;
	double					delta;
	int					extra_ticks; //Keeps track of how many ticks we have to add to keep account with encoder overflows
	bool					overflowup;
	bool					overflowdown;

    public:
        Encoder(std::string name);
        ~Encoder();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
        
    };
}
#endif // __ENCODER__
