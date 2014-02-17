#include "planeSimulatorRframe.hpp"
#include "model/rk4_rframe.h"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::planeSimulatorRframe)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
     planeSimulatorRframe::planeSimulatorRframe(std::string name)
         : TaskContext(name)
     {
        // Add properties

        properties()->addProperty( "prop",_prop )
			.doc("The description of the property");

	// Add ports
	addEventPort( "trigger",_trigger);

        addPort( "controlInputPort",_controlInputPort )
			.doc("Input to the system: dur1,dur2,dup");
        addPort( "stateOutputPort",_stateOutputPort )
			.doc("State output: x,y,z,dx,dy,dz,e11,e12,e13,e21,e22,e23,e31,e32,e33,w1,w2,w3,delta,ddelta,ua,ue");
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

	provides()->addOperation("initialiseState",&planeSimulatorRframe::initialiseState,this)
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
    

    planeSimulatorRframe::~planeSimulatorRframe()
    {
    }

    bool  planeSimulatorRframe::configureHook()
    {
        return true;
     }

    bool  planeSimulatorRframe::startHook()
    {
	X[0] = 1.192115285355252;
	X[1] = -0.06866810211585969;
	X[2] = -0.1189362777884522;
	X[3] = 0;
	X[4] = 0;
	X[5] = 0;
	X[6] = 0.09114325577860947;
	X[7] = 0.0605108135794719;
	X[8] = 0.993997660141122;
	X[9] = 0.9958127859597069;
	X[10] = -0.01261153628993693;
	X[11] = -0.09054194868443322;
	X[12] = 0.007057070585016309;
	X[13] = 0.9980878671701996;
	X[14] = -0.06140689832909731;
	X[15] = 0.04434098589977448;
	X[16] = 6.271185687003798;
	X[17] = -0.3858318235813842;
	X[18] = 0;
	X[19] = 6.2832;
	X[20] = 0;
	X[21] = 0;
#if 0
	X[0] =     1.1640;
	X[1] =    -0.1107;
	X[2] =    -0.2700;
	X[3] =          0;
	X[4] =          0;
	X[5] =          0;
	X[6] =     0.1244;
	X[7] =     0.3631;
	X[8] =     0.9234;
	X[9] =     0.9914;
	X[10] =    -0.0837;
	X[11] =    -0.1007;
	X[12] =     0.0408;
	X[13] =     0.9280;
	X[14] =    -0.3704;
	X[15] =     0.1922;
	X[16] =     4.3731;
	X[17] =    -1.7452;
	X[18] =          0;
	X[19] =     4.7124;
	X[20] =    -0.0066;
	X[21] =          0;
#endif
  	for(int i=22; i<NSTATES+NIMU;i++){X[i] = 0.0;}
	//_stateOutputPort.write(X);
        return true;
    }

    void  planeSimulatorRframe::updateHook()
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
	
	int N = 5;
	for(int i=0; i<N; i++){
		rk4Wrapper(X, U, h/N, &X1,&IMU);
		X=X1;
	}
	// Add noise to IMU measurement
	for(unsigned int i=0; i<NIMU; i++){
		//IMU[i] += (drand48()-0.5);
	}

	deltaOmega[0] = X[18];
	deltaOmega[1] = X[19];
	controlOutputPort[0] = SCALE_UR*X[20];
	controlOutputPort[1] = SCALE_UR*X[20];
	controlOutputPort[2] = SCALE_UP*X[21];
	_stateOutputPort.write(X);
	_triggerTimeStampOut.write(triggerTimeStamp);
	for(unsigned int i=0; i<3; i++){
		IMU[i] += (drand48()-0.5)*0.05;
		IMU[i+3] += (drand48()-0.5)*0.5;
	}
	_IMUOutputPort.write(IMU);
	_controlOutputPort.write(controlOutputPort);
	_deltaOmega.write(deltaOmega);
	//cout << "The simulation took " << os::TimeService::Instance()->secondsSince(time_begin) << " seconds." << endl;
    }

    void  planeSimulatorRframe::stopHook()
    {
    }

    void  planeSimulatorRframe::cleanUpHook()
    {
    }

    int planeSimulatorRframe::initialiseState(vector<double> X0){
	X = X0;
	return 0;
    }

	int planeSimulatorRframe::rk4Wrapper(vector<double> X, vector<double> U, double h, vector<double> *X1, vector<double> *IMU){
		copy(X.begin(), X.end(), rk4Wrapper_input[0]);
		copy(U.begin(), U.end(), rk4Wrapper_input[1]);
		rk4Wrapper_input[2][0] = h;
		if( rk4_rframe(rk4Wrapper_input, rk4Wrapper_output) ){ return 1;}
		(*X1).assign(rk4Wrapper_output[0],rk4Wrapper_output[0]+NSTATES);
		(*IMU).assign(rk4Wrapper_output[1],rk4Wrapper_output[1]+NIMU);
		return 0;
	}


}//namespace

