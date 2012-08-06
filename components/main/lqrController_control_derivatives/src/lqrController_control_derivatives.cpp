#include "lqrController_control_derivatives.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::LqrController_control_derivatives )

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
	 LqrController_control_derivatives::LqrController_control_derivatives(std::string name) : TaskContext(name)
	 {
		ports()->addPort( "stateInputPort",_stateInputPort ).doc("x,y,z"
					 ",dx,dy,dz"
					 ",e11,e12,e13,e21,e22,e23,e31,e32,e33"
					 ",w1,w2,w3"
					 ",delta,ddelta,ur,up"); // We ONLY use the first 18 and lost 2 states from this.

		ports()->addPort( "controlInputPort",_controlInputPort ).doc("ua1,ua2,ue");
		ports()->addPort( "controlOutputPort",_controlOutputPort ).doc("ua1,ua2,ue");
		ports()->addPort( "controlRatesOutputPort",_controlRatesOutputPort).doc("dur1,dur2,dup");
		ports()->addPort( "errorOutputPort",_errorOutputPort ).doc("Same size as X and Xref");
		ports()->addPort( "Xref",_Xref ).doc("The Xref that is currently being used");
		ports()->addPort( "Uref",_Uref ).doc("The Uref that is currently being used");
		ports()->addPort( "K",_K ).doc("Gain Matrix that is currently being applied");

		ports()->addEventPort( "mhePortReady",_mhePortReady ).doc("For checking if the MHE port is ready");

		provides()->addOperation("loadGains",&LqrController_control_derivatives::loadGains,this).doc("Reload LQR gains and refernces.");
		provides()->addOperation("changeRef",&LqrController_control_derivatives::changeRef,this).doc("Reload LQR gains and refernces.");
		addProperty("dt",dt).doc("time step");
		dt = 0.1;

		X.resize(NSTATES,0.0);
		Xref.resize(NSTATES,0.0);
		U.resize(NOUTPUTS,0.0); // three, to match our output size.
		dU.resize(NOUTPUTS,0.0); // three, to match our output size.
		Uref.resize(NOUTPUTS,0.0); // only two, to match the file we read in!
		U_scaled.resize(NOUTPUTS,0.0);
		dU_scaled.resize(NOUTPUTS,0.0);
		E.resize(NSTATES,0.0);
		controlRatesOutput.resize(3,0.0);
	 }

	 void LqrController_control_derivatives::loadGains()
	 {
		loadVectorFromDat(XREF_FILENAME,Xref);
		loadMatrixFromDat(K_FILENAME,K);
		stop();
	 }

	LqrController_control_derivatives::~LqrController_control_derivatives()
	{
	}

	bool  LqrController_control_derivatives::configureHook()
	{
		loadGains();
		_controlOutputPort.write(U);
		
		return true;
	 }

	bool  LqrController_control_derivatives::startHook()
	{
		return true;
	}
			
	void  LqrController_control_derivatives::updateHook()
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
			// Use the control that we applied at previous sampling time, and not the one that was estimated by the MHE.
			_controlInputPort.read(U_scaled);
			U[0] = U_scaled[0]/SCALE_UR;
			U[1] = U[0];
			U[2] = U_scaled[2]/SCALE_UP;
			X[20] = U[0];
			X[21] = U[2];
			for(unsigned int i=0; i<X.size(); i++)	
			{
				E[i]=X[i]-Xref[i];
			}

			// Compute U = -E*K + Uref
			dU[0]=0;
			dU[1]=0;
			dU[2]=0;
			for(unsigned int i=0; i<KSTATES-2; i++)	
			{
				dU[0] += -E[i]*K[0][i];
				dU[2] += -E[i]*K[1][i];
			}
			for(unsigned int i=0; i<2; i++)	// Also for control values
			{
				dU[0] += -E[KSTATES+i]*K[0][KSTATES-2+i];
				dU[2] += -E[KSTATES+i]*K[1][KSTATES-2+i];
			}
			 // Now we have control derivative. Compute now the actual control action via first order euler
			U[0] = X[20] + dt*dU[0];
			U[1] = U[0]; // We aren't doing differential ailerons, so need to duplicate other aileron value.
			U[2] = X[21] + dt*dU[2];

			U_scaled[0] = U[0]*SCALE_UR;
			U_scaled[1] = U[1]*SCALE_UR;
			U_scaled[2] = U[2]*SCALE_UP;
			dU_scaled[0] = dU[0]*SCALE_UR;
			dU_scaled[1] = dU[0]*SCALE_UR;
			dU_scaled[2] = dU[2]*SCALE_UP;

			_errorOutputPort.write(E);
			_controlOutputPort.write(U_scaled); // Stuff should trigger on this
			_controlRatesOutputPort.write(dU_scaled);
		}
	}

	void  LqrController_control_derivatives::stopHook()
	{
		U[0] = 0.0;
		U[1] = 0.0;
		U[2] = 0.0;
		_controlOutputPort.write(U);
	}

	void  LqrController_control_derivatives::cleanUpHook()
	{
	}

	 void LqrController_control_derivatives::loadVectorFromDat(const char* filename, vector<double> &V)
	 {
		 ifstream inFile;
		 inFile.open(filename, ios::in);
		 if(!inFile)
		 {
			 cout <<  "(lqrController_control_derivatives): Unable to read in vector from " << filename << endl;
		 } else {
			 for(unsigned int i=0; i<V.size(); i++)
			 {
				 inFile >> V[i];
			 }
		 }
		 inFile.close();
	 };

	void LqrController_control_derivatives::loadMatrixFromDat(const char* filename, vector<vector<double> > &V)
	{
		ifstream controlFile( filename );
		string line;
		if ( controlFile.is_open() )
		{
			while( getline(controlFile, line) )
			{
				istringstream linestream( line );
				vector< double > linedata;
				double number;

				while( linestream >> number )
				{
					linedata.push_back( number );
				}

				V.push_back( linedata );
			}

			controlFile.close();
		}
		else
		{
			cout << "File " << filename << " could not be opened" << endl;
		}
	};

	void LqrController_control_derivatives::changeRef(int ref){
		vector<double> Xref_new;
		Xref_new.resize(NSTATES,0.0);
		vector<vector<double> > K_new;
		std::ostringstream oss_X;
		oss_X << STUPID_LONG_PATH << "Xref" << ref << ".dat";
		std::ostringstream oss_K;
		oss_K << STUPID_LONG_PATH << "K" << ref << ".dat";
		loadVectorFromDat(oss_X.str().c_str(),Xref_new);
		loadMatrixFromDat(oss_K.str().c_str(),K_new);
		K = K_new;
		Xref = Xref_new;
	}

}//namespace

