#ifndef __LQRCONTROLLER__
#define __LQRCONTROLLER__

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

#define SCALE_UR	1.25e6	// Scaling of ur_1, ur_2
#define SCALE_UP	2e5		// Scaling of u_p

#define STUPID_LONG_PATH "/home/planepower/Work/SVN/PLANEPOWER/TRUNK/orocos/components/main/lqrController/src/code/"
#define K_FILENAME STUPID_LONG_PATH "K.dat"
#define XREF_FILENAME STUPID_LONG_PATH "x_ref.dat"
#define UREF_FILENAME STUPID_LONG_PATH "u0.dat"

#define XREF1_FILENAME STUPID_LONG_PATH "x_ref1.dat"
#define UREF1_FILENAME STUPID_LONG_PATH "u01.dat"

#define NSTATES 22
#define KSTATES 18 // Note: K doesn't have the last few entries corresponding to (estimated) controls
#define NOUTPUTS 3 // The number of outputs of this component

namespace OCL
{
    class LqrController
        : public TaskContext
    {
    protected:
        InputPort<vector<double> >		_stateInputPort;
        OutputPort<vector<double> >		_controlOutputPort;
        OutputPort<vector<double> >		_errorOutputPort;
        OutputPort<vector<double> >		_Xref;
        OutputPort<vector<double> >		_Uref;
        OutputPort<vector<double> >		_K;   
		InputPort<bool>				_mhePortReady;
		bool					mhePortReady;

    private:
		void					simpleFunction(void);
		vector<double>			X; //The state of the system, augmented with the accelerations
		vector<double>			Xref; // Reference value for X
		vector<double>			E; //The state of the system, augmented with the accelerations
		vector<double>			U; //The controls of the system
		vector<double>			Uref; //The controls of the system
		vector<double>			K; // The gain matrix, row-major storage
		void				loadVectorFromDat(const char *filename, vector<double> &V);
		void				changeReference(int ref);

    public:
        LqrController(std::string name);
        ~LqrController();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
		void		loadGains();
    };
}
#endif // __LQRCONTROLLER__
