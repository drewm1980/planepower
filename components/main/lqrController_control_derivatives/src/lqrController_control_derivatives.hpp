#ifndef __LQRCONTROLLER_CONTROL_DERIVATIVES__
#define __LQRCONTROLLER_CONTROL_DERIVATIVES__

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

#define STUPID_LONG_PATH "/home/planepower/Work/SVN/PLANEPOWER/TRUNK/orocos/components/main/lqrController_control_derivatives/src/discrete_LQR_udot_ver1/"
#define K_FILENAME STUPID_LONG_PATH "K.dat"
#define XREF_FILENAME STUPID_LONG_PATH "x_ref.dat"
#define UREF_FILENAME STUPID_LONG_PATH "u0.dat"

#define NSTATES 22 // state from MHE = [x;y;z;dx;dy;dz;e11;e12;e13;e21;e22;e23;e31;e32;e33;w1;w2;w3;delta;ddelta;ur,up]  
				   // For sanity...    [1;2;3;4;5 ;6 ;7 ;8   ;9  ;10 ; 11;12 ;13; 14 ;15 ;16;17;18;19   ;20    ;21,22]  
#define KSTATES 20 // delta and ddelta isn't part of the state that we actually use for control:
					// state we control = [x;y;z;dx;dy;dz;e11;e12;e13;e21;e22;e23;e31;e32;e33;w1;w2;w3;ur,up]  
				      // For sanity...    [1;2;3;4 ;5 ;6 ;7 ;8   ;9  ;10 ; 11;12 ;13 ;14 ;15 ;16;17;18;19;20]  
#define NOUTPUTS 3 // The number of outputs of this component

namespace OCL
{
    class LqrController_control_derivatives
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
		vector<double>			dU; //The control derivatives of the system
		vector<double>			U_scaled;
		vector<double>			Uref; //The controls of the system
		vector<double>			K; // The gain matrix, row-major storage
		void				loadVectorFromDat(const char *filename, vector<double> &V);
		double				dt;

    public:
        LqrController_control_derivatives(std::string name);
        ~LqrController_control_derivatives();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
		void		loadGains();
    };
}
#endif // __LQRCONTROLLER__
