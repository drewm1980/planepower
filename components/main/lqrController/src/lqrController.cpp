#include "lqrController.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::LqrController)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
     LqrController::LqrController(std::string name) : TaskContext(name)
	 {
		ports()->addPort( "stateInputPort",_stateInputPort ).doc("x,y,z"
					 ",dx,dy,dz"
					 ",e11,e12,e13,e21,e22,e23,e31,e32,e33"
					 ",w1,w2,w3"
					 ",delta,ddelta,ur,up"); // We ONLY use the first 18 states from this;
											 // mismatch in u has no effect on us.

		ports()->addPort( "controlOutputPort",_controlOutputPort ).doc("ua1,ua2,ue");
		ports()->addPort( "errorOutputPort",_errorOutputPort ).doc("Same size as X and Xref");
		ports()->addPort( "Xref",_Xref ).doc("The Xref that is currently being used");
		ports()->addPort( "Uref",_Uref ).doc("The Uref that is currently being used");
		ports()->addPort( "K",_K ).doc("Gain Matrix that is currently being applied");

		ports()->addEventPort( "mhePortReady",_mhePortReady ).doc("For checking if the MHE port is ready");

		provides()->addOperation("loadGains",&LqrController::loadGains,this).doc("Reload LQR gains and refernces.");
		provides()->addOperation("changeReference",&LqrController::changeReference,this).doc("Change the reference.");

		 X.resize(NSTATES,0.0);
		 Xref.resize(NSTATES,0.0);
		 U.resize(NOUTPUTS,0.0); // three, to match our output size.
		 Uref.resize(NOUTPUTS,0.0); // only two, to match the file we read in!
		 E.resize(NSTATES,0.0);
		 K.resize(2*KSTATES,0.0);
	 }

	 void LqrController::loadVectorFromDat(const char* filename, vector<double> &V)
	 {
		 ifstream inFile;
		 inFile.open(filename, ios::in);
		 if(!inFile)
		 {
			 cout <<  "(lqrController): Unable to read in vector from " << filename << endl;
		 } else {
			 for(unsigned int i=0; i<V.size(); i++)
			 {
				 inFile >> V[i];
			 }
		 }
		 inFile.close();
	 };

	 void LqrController::loadGains()
	 {
		 loadVectorFromDat(XREF_FILENAME,Xref);
		 loadVectorFromDat(UREF_FILENAME,Uref);
		 loadVectorFromDat(K_FILENAME,K);
		 stop();
	 }

    LqrController::~LqrController()
    {
    }

    bool  LqrController::configureHook()
    {
		loadGains();
		_controlOutputPort.write(U);
        
        return true;
     }

    bool  LqrController::startHook()
    {
        return true;
    }

    void  LqrController::updateHook()
    {
		// Write the gains and references we're using
		// for online debugging, especially if they're changed during flight.
		_Xref.write(Xref);
		_Uref.write(Uref);
		_K.write(K);   

		//checking is if MHE is ready
		_mhePortReady.read(mhePortReady);
		if (mhePortReady == true)
		{
			_stateInputPort.read(X);
			for(unsigned int i=0; i<X.size(); i++)	
			{
				E[i]=X[i]-Xref[i];
			}

			// Compute U = -E*K + Uref
			U[0]=0;
			U[1]=0;
			U[2]=0;
			for(unsigned int i=0; i<KSTATES; i++)	
			{
				U[0] += -E[i]*K[i];
				U[2] += -E[i]*K[i+KSTATES];
			}
			U[0] += Uref[0];
			U[1] = U[0]; // We aren't doing differential ailerons, so need to duplicate other aileron value.
			U[2] += Uref[1];

			U[0] *= SCALE_UR;
			U[1] *= SCALE_UR;
			U[2] *= SCALE_UP;

			_errorOutputPort.write(E);
			_controlOutputPort.write(U); // Stuff should trigger on this
		}
    }

    void  LqrController::stopHook()
    {
		U[0] = 0.0;
		U[1] = 0.0;
		U[2] = 0.0;
		_controlOutputPort.write(U);
    }

    void LqrController::changeReference(int ref)
    {
	if(ref == 0){
		 loadVectorFromDat(XREF_FILENAME,Xref);
		 loadVectorFromDat(UREF_FILENAME,Uref);
	}
	else if(ref == 1){
		 loadVectorFromDat(XREF1_FILENAME,Xref);
		 loadVectorFromDat(UREF1_FILENAME,Uref);
	}
	else{
		cout << "Reference is not defined. Must be 0 or 1" << endl;
	}
    }

    void  LqrController::cleanUpHook()
    {
    }

}//namespace

