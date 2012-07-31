#include "planeSimulatorDirectControls.hpp"
#include <ocl/Component.hpp>
#include "model/rk4.c"

ORO_CREATE_COMPONENT( OCL::planeSimulatorDirectControls)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
     planeSimulatorDirectControls::planeSimulatorDirectControls(std::string name)
         : TaskContext(name)
     {
        log(Debug) << "(planeSimulatorDirectControls) constructor entered" << endlog();

        // Add properties

        properties()->addProperty( "prop",_prop ).doc("The description of the property");

	// Add ports

        ports()->addPort( "controlInputPort",_controlInputPort ).doc("Input to the system: dddelta,ddr,ua,ue");
        ports()->addPort( "stateOutputPort",_stateOutputPort ).doc("State output: x,y,z,dx,dy,dz,e11,e12,e13,e21,e22,e23,e31,e32,e33,w1,w2,w3,r,dr,delta,ddelta,ax,ay,az");

	// Add operations

	provides()->addOperation("initialiseState",&planeSimulatorDirectControls::initialiseState,this).doc("Description of the method");

	X.resize(27);
	U.resize(4);
	X_reduced.resize(25);
	U_direct.resize(4);
	full_array[0] = X_array;
	full_array[1] = U_array;
	full_array[2] = h_array;

	X1_big_array[0] = X1_array;

        log(Debug) << "(planeSimulatorDirectControls) constructor finished" << endlog();
    }
    

    planeSimulatorDirectControls::~planeSimulatorDirectControls()
    {
    }

    bool  planeSimulatorDirectControls::configureHook()
    {
        log(Debug) << "(planeSimulatorDirectControls) ConfigureHook entered" << endlog();

        log(Debug) << "(planeSimulatorDirectControls) ConfigureHook finished" << endlog();
        
        return true;
     }

    bool  planeSimulatorDirectControls::startHook()
    {
        log(Debug) << "(planeSimulatorDirectControls) startHook entered" << endlog();

	h = getPeriod();
	full_array[2][0] = h;
	X[0] = 0.0805096637;
	X[1] = 2.2622502684;
	X[2] = 0.0;
	X[3] = -14.2141708865;
	X[4] = 0.5058583188;
	X[5] = 0.0;
	X[6] = -0.9959802258;
	X[7] = -0.0184245091;
	X[8] = 0.0876580162;
	X[9] = 0.0887730986;
	X[10] = -0.0725115593;
	X[11] = 0.9934089849;
	X[12] = -0.0119468537;
	X[13] = 0.9971973784;
	X[14] = 0.0738556797;
	X[15] = -0.0750644714;
	X[16] = 6.2655905679;
	X[17] = 0.4640500070;
	X[18] = 1.1800000000;
	X[19] = 0.0000000000;
	X[20] = 0.0000000000;
	X[21] = 6.2832000000;
	X_reduced[0] = X[0];
	X_reduced[1] = X[1];
	X_reduced[2] = X[2];
	X_reduced[3] = X[3];
	X_reduced[4] = X[4];
	X_reduced[5] = X[5];
	X_reduced[6] = X[6];
	X_reduced[7] = X[7];
	X_reduced[8] = X[8];
	X_reduced[9] = X[9];
	X_reduced[10] = X[10];
	X_reduced[11] = X[11];
	X_reduced[12] = X[12];
	X_reduced[13] = X[13];
	X_reduced[14] = X[14];
	X_reduced[15] = X[15];
	X_reduced[16] = X[16];
	X_reduced[17] = X[17];
	X_reduced[18] = X[18];
	X_reduced[19] = X[19];
	X_reduced[20] = X[20];
	X_reduced[21] = X[21];
	X_reduced[22] = X[24];
	X_reduced[23] = X[25];
	X_reduced[24] = X[26];
	//cout << "states: " << endl;
	//cout << X[0] << endl;
	_stateOutputPort.write(X_reduced);

        log(Debug) << "(planeSimulatorDirectControls) startHook finished" << endlog();
        return true;
    }

    void  planeSimulatorDirectControls::updateHook()
    {
        log(Debug) << "(planeSimulatorDirectControls) updateHook entered" << endlog();
	time_begin= os::TimeService::Instance()->getTicks();
	
	// Copy the value from the vectors X and U to the arrays X and U
	// Read the control input from the port
	_controlInputPort.read(U_direct);
	// Control inputs from ports are aileron and elevator positions. These are states in Mario's model...
	X[22] = U_direct[2];
	X[23] = U_direct[3];
	U[0] = U_direct[0];
	U[1] = U_direct[1];
	U[2] = 0.0;
	U[3] = 0.0;
	//cout << U_direct[0] << endl;
	//cout << U_direct[1] << endl;
	//cout << U_direct[2] << endl;
	//cout << U_direct[3] << endl;
	copy( X.begin(), X.end(), full_array[0]);
	copy( U.begin(), U.end(), full_array[1]);
	// Apply the control through the model
	if( evaluate(full_array,X1_big_array) ){printf("Something went wrong: evaulate did not complete succesfully \n");}

	// Assign the output to array X
	full_array[0] = X1_big_array[0];
	X.assign(X1_big_array[0],X1_big_array[0]+27);
	// Write the update state on the port
	X_reduced[0] = X[0];
	X_reduced[1] = X[1];
	X_reduced[2] = X[2];
	X_reduced[3] = X[3];
	X_reduced[4] = X[4];
	X_reduced[5] = X[5];
	X_reduced[6] = X[6];
	X_reduced[7] = X[7];
	X_reduced[8] = X[8];
	X_reduced[9] = X[9];
	X_reduced[10] = X[10];
	X_reduced[11] = X[11];
	X_reduced[12] = X[12];
	X_reduced[13] = X[13];
	X_reduced[14] = X[14];
	X_reduced[15] = X[15];
	X_reduced[16] = X[16];
	X_reduced[17] = X[17];
	X_reduced[18] = X[18];
	X_reduced[19] = X[19];
	X_reduced[20] = X[20];
	X_reduced[21] = X[21];
	X_reduced[22] = X[24];
	X_reduced[23] = X[25];
	X_reduced[24] = X[26];
	//cout << "states: " << endl;
	//cout << X[0] << endl;
	//cout << U_direct[2] << endl;
	//cout << U_direct[3] << endl;
	_stateOutputPort.write(X_reduced);

        log(Debug) << "(planeSimulatorDirectControls) updateHook finished" << endlog();
    }

    void  planeSimulatorDirectControls::stopHook()
    {
        log(Debug) << "(planeSimulatorDirectControls) stopHook entered" << endlog();

        //Put here code that is needed to stop the component

        log(Debug) << "(planeSimulatorDirectControls) stopHook finished" << endlog();
    }

    void  planeSimulatorDirectControls::cleanUpHook()
    {
        log(Debug) << "(planeSimulatorDirectControls) cleanupHook entered" << endlog();

        //Put here code that is needed to clean up the component

        log(Debug) << "(planeSimulatorDirectControls) cleanupHook finished" << endlog();
    }

    int planeSimulatorDirectControls::initialiseState(vector<double> X0){
	X = X0;
	return 0;
    }


}//namespace

