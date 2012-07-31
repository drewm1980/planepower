#include "playControls.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::PlayControls)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 PlayControls::PlayControls(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

		// Add ports

		ports()->addPort( "controlOutputPort",_controlOutputPort ).doc("ua1,ua2,ue");

		// Add operations


		U.resize(3);
	}
	

	PlayControls::~PlayControls()
	{
	}

	bool  PlayControls::configureHook()
	{
		return true;
	 }

	bool  PlayControls::startHook()
	{
		loadVectorFromDat("/home/planepower/Work/SVN/PLANEPOWER/TRUNK/orocos/components/main/playControls/TU.txt",TU);
		for(unsigned int i = 0; i < TU.size(); i++){
			cout << TU[i][0] << endl;
		}
		TU_i = 0;
		return true;
	}

	void  PlayControls::updateHook()
	{
		if(TU_i < TU.size()){
			U[0] = U[1] = TU[TU_i][1];
			U[2] = TU[TU_i][2];
			_controlOutputPort.write(U);
			TU_i++;
		}
		else{
			U[0] = U[1] = U[2] = 0.0;
			_controlOutputPort.write(U);
		}
	}

	void  PlayControls::stopHook()
	{
	}

	void  PlayControls::cleanUpHook()
	{
	}

	void PlayControls::loadVectorFromDat(const char* filename, vector<vector<double> > &V)
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


}//namespace

