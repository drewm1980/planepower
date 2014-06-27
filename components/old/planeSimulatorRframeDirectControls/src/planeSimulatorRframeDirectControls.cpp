#include "planeSimulatorRframeDirectControls.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::planeSimulatorRframeDirectControls)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
     planeSimulatorRframeDirectControls::planeSimulatorRframeDirectControls(std::string name)
         : TaskContext(name)
     {
        // Add properties

        properties()->addProperty( "prop",_prop )
			.doc("The description of the property");

	// Add ports
	addEventPort( "trigger",_trigger);

        addPort( "controlInputPort",_controlInputPort )
			.doc("Input to the system: ur1,ur2,up");
        addPort( "stateOutputPort",_stateOutputPort )
			.doc("State output: x,y,z,dx,dy,dz,e11,e12,e13,e21,e22,e23,e31,e32,e33,w1,w2,w3,delta,ddelta");
	addPort( "triggerTimeStampOut",_triggerTimeStampOut )
			.doc("triggerTimeStampOut");
	addPort( "imuData",_IMUOutputPort )
			.doc("IMU output: wx,wy,wz,ax,ay,az");
	addPort( "deltaOmega",_deltaOmega )
			.doc("encoder output: delta, ddelta");
	deltaOmega.resize(2);
	addPort( "controlOutputPort",_controlOutputPort )
			.doc("controlOutputPort: ur1, ur2, up");
	controlOutputPort.resize(3);

	addProperty( "integrationStep",h ).doc("The time step to integrate");
	h = 1.0/500.0;

	// Add operations

	provides()->addOperation("initialiseState",&planeSimulatorRframeDirectControls::initialiseState,this)
		.doc("Description of the method");

	X.resize(NSTATES);
	X1.resize(NSTATES);
	IMU.resize(NIMU);
	U.resize(NCONTROLS);
	rk4Wrapper_input[0] = rk4Wrapper_input_X;
	rk4Wrapper_input[1] = rk4Wrapper_input_U;
	rk4Wrapper_input[2] = rk4Wrapper_input_h;

	rk4Wrapper_output[0] = rk4Wrapper_output_X1;
	rk4Wrapper_output[1] = rk4Wrapper_output_IMU;
    }
    

    planeSimulatorRframeDirectControls::~planeSimulatorRframeDirectControls()
    {
    }

    bool  planeSimulatorRframeDirectControls::configureHook()
    {
        return true;
     }

    bool  planeSimulatorRframeDirectControls::startHook()
    {	X[0] = 1.1916;
	X[1] = -0.070538;
	X[2] = -0.12333;
	X[3] = -0.001932;
	X[4] = 0.026007;
	X[5] = -0.033537;
	X[6] = 0.090486;
	X[7] = 0.049386;
	X[8] = 0.99467;
	X[9] = 0.99589;
	X[10] = -0.0014043;
	X[11] = -0.090527;
	X[12] = -0.003074;
	X[13] = 0.99878;
	X[14] = -0.049311;
	X[15] = -0.0013158;
	X[16] = 6.2889;
	X[17] = -0.26363;
	X[18] = 0.0;
	X[19] = 6.2832;

	X[0] = 1.1254;
	X[1] = -0.1911;
	X[2] = -0.3700;
	X[3] = 0;
	X[4] = 0;
	X[5] = 0;
	X[6] = 0.1257;
	X[7] = 0.5444;
	X[8] = 0.8294;
	X[9] = 0.9876;
	X[10] = -0.1477;
	X[11] = -0.0528;
	X[12] = 0.0938;
	X[13] = 0.8257;
	X[14] = -0.5562;
	X[15] = 0.3634;
	X[16] = 3.1994;
	X[17] = -2.1551;
	X[18] = 0;
	X[19] = 6.3;

  	for(int i=NSTATES; i<NSTATES+NIMU;i++){X[i] = 0.0;}
	//_stateOutputPort.write(X);
        return true;
    }

    void  planeSimulatorRframeDirectControls::updateHook()
    {
	//time_begin= os::TimeService::Instance()->getTicks();
	if(_trigger.read(triggerTimeStamp) == NoData){
		triggerTimeStamp=os::TimeService::Instance()->getTicks();
	}
	// Read the control input from the port
	if(_controlInputPort.read(U)==NoData){U[0] = U[1] = U[2] = 0.0;}
	U[0] = 0.0; // corresponds to dddelta, which is not yet controlled
	U[1] = U[1]/SCALE_UR;
	U[2] = U[2]/SCALE_UP;
	

	rk4Wrapper(X, U, h, &X1,&IMU);
	X=X1;
	// Add noise to IMU measurement
	for(unsigned int i=0; i<NIMU; i++){
		//IMU[i] += (drand48()-0.5);
	}

	deltaOmega[0] = X[18];
	deltaOmega[1] = X[19];
	controlOutputPort[0] = SCALE_UR*U[1];
	controlOutputPort[1] = SCALE_UR*U[1];
	controlOutputPort[2] = SCALE_UP*U[2];
	_stateOutputPort.write(X);
	_triggerTimeStampOut.write(triggerTimeStamp);
	_IMUOutputPort.write(IMU);
	_controlOutputPort.write(controlOutputPort);
	_deltaOmega.write(deltaOmega);
	//cout << "The simulation took " << os::TimeService::Instance()->secondsSince(time_begin) << " seconds." << endl;
    }

    void  planeSimulatorRframeDirectControls::stopHook()
    {
    }

    void  planeSimulatorRframeDirectControls::cleanUpHook()
    {
    }

    int planeSimulatorRframeDirectControls::initialiseState(vector<double> X0){
	X = X0;
	return 0;
    }

	int planeSimulatorRframeDirectControls::rk4Wrapper(vector<double> X, vector<double> U, double h, vector<double> *X1, vector<double> *IMU){
		copy(X.begin(), X.end(), rk4Wrapper_input[0]);
		copy(U.begin(), U.end(), rk4Wrapper_input[1]);
		rk4Wrapper_input[2][0] = h;
		if( rk4(rk4Wrapper_input, rk4Wrapper_output) ){ return 1;}
		(*X1).assign(rk4Wrapper_output[0],rk4Wrapper_output[0]+NSTATES);
		(*IMU).assign(rk4Wrapper_output[1],rk4Wrapper_output[1]+NIMU);
		return 0;
	}


}//namespace

