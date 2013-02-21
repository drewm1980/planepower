#include "planeSimulator.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::planeSimulator)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

extern int rk4( double** x, double** r);

namespace OCL
{
     planeSimulator::planeSimulator(std::string name)
         : TaskContext(name)
     {
        log(Debug) << "(planeSimulator) constructor entered" << endlog();

        // Add properties

        properties()->addProperty( "prop",_prop )
			.doc("The description of the property");

	// Add ports

        ports()->addPort( "controlInputPort",_controlInputPort )
			.doc("Input to the system: dddelta,ddr,dua,due");
        ports()->addPort( "stateOutputPort",_stateOutputPort )
			.doc("State output: x,y,z"
								",dx,dy,dz"
								",e11,e12,e13"
								",e21,e22,e23"
								",e31,e32,e33"
								",w1,w2,w3"
								",r,dr,delta,ddelta"
								",ua,ue,ax,ay,az");

	// Add operations

	provides()->addOperation("initialiseState",&planeSimulator::initialiseState,this)
		.doc("Description of the method");

	X.resize(27);
	U.resize(4);
	full_array[0] = X_array;
	full_array[1] = U_array;
	full_array[2] = h_array;

	X1_big_array[0] = X1_array;

        log(Debug) << "(planeSimulator) constructor finished" << endlog();
    }
    

    planeSimulator::~planeSimulator()
    {
    }

    bool  planeSimulator::configureHook()
    {
        log(Debug) << "(planeSimulator) ConfigureHook entered" << endlog();

        log(Debug) << "(planeSimulator) ConfigureHook finished" << endlog();
        
        return true;
     }

    bool  planeSimulator::startHook()
    {
        log(Debug) << "(planeSimulator) startHook entered" << endlog();

	h = 0.001;
	full_array[2][0] = h;
	X[0] = 1.052162812025321248e-01;
	X[1] = 2.260299763536906870e+00;
	X[2] = 0.000000000000000000e+00;
	X[3] = -7.100941132038292025e+00;
	X[4] = 3.305466960639127372e-01;
	X[5] = 0.000000000000000000e+00;
	X[6] = -9.894326017588626065e-01;
	X[7] = -5.314856757372120322e-02;
	X[8] = 1.349012836912586166e-01;
	X[9] = 1.431089684196779066e-01;
	X[10] = -2.084508131761203253e-01;
	X[11] = 9.675061145254172379e-01;
	X[12] = -2.330128182189722397e-02;
	X[13] = 9.765876756600381237e-01;
	X[14] = 2.138540670980581260e-01;
	X[15] = -7.320313579089771094e-02;
	X[16] = 3.068040667439907221e+00;
	X[17] = 6.718423661355581444e-01;
	X[18] = 1.179999999999999938e+00;
	X[19] = -0.000000000000000000e+00;
	X[20] = 0.000000000000000000e+00;
	X[21] = 3.141592653589792672e+00;
	X[22] = -1.376317243483962107e-03;
	X[23] = -6.975489828669869730e-05;
	_stateOutputPort.write(X);

        log(Debug) << "(planeSimulator) startHook finished" << endlog();
        return true;
    }

    void  planeSimulator::updateHook()
    {
        log(Debug) << "(planeSimulator) updateHook entered" << endlog();
	time_begin= os::TimeService::Instance()->getTicks();
	
	// Copy the value from the vectors X and U to the arrays X and U
	copy( X.begin(), X.end(), full_array[0]);
	// Read the control input from the port
	_controlInputPort.read(U);
	copy( U.begin(), U.end(), full_array[1]);\
	// Apply the control through the model
	if( rk4(full_array,X1_big_array) ){printf("Something went wrong: evaulate did not complete succesfully \n");}
	// Assign the output to array X
	full_array[0] = X1_big_array[0];
	// Copy array X to vector X
	X.assign(full_array[0],full_array[0]+27);
	// Write the update state on the port
	_stateOutputPort.write(X);
	cout << "The simulation took " << os::TimeService::Instance()->secondsSince(time_begin) << " seconds." << endl;

        log(Debug) << "(planeSimulator) updateHook finished" << endlog();
    }

    void  planeSimulator::stopHook()
    {
        log(Debug) << "(planeSimulator) stopHook entered" << endlog();

        //Put here code that is needed to stop the component

        log(Debug) << "(planeSimulator) stopHook finished" << endlog();
    }

    void  planeSimulator::cleanUpHook()
    {
        log(Debug) << "(planeSimulator) cleanupHook entered" << endlog();

        //Put here code that is needed to clean up the component

        log(Debug) << "(planeSimulator) cleanupHook finished" << endlog();
    }

    int planeSimulator::initialiseState(vector<double> X0){
	X = X0;
	return 0;
    }


}//namespace

