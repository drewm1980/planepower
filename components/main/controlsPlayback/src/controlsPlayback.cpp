#include "controlsPlayback.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ControlsPlayback)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 ControlsPlayback::ControlsPlayback(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

		// Add ports

		ports()->addPort( "controlOutputPort",_controlOutputPort ).doc("Control output: ur1, ur2, up");
		ports()->addEventPort( "triggerTimeStampIn",_triggerTimeStampIn).doc("triggers updateHook");

		// Add operations
		provides()->addOperation("startSequence",&ControlsPlayback::startSequence,this).doc("Start the sequence. It will keep going until you stop it!");
		provides()->addOperation("stopSequence",&ControlsPlayback::stopSequence,this).doc("Stop the sequence");

		sequencing = false;
		controlOutput.resize(3,0.0);
		_controlOutputPort.write(controlOutput);
	}
	

	ControlsPlayback::~ControlsPlayback()
	{
	}

	bool  ControlsPlayback::configureHook()
	{
		return true;
	 }

	bool  ControlsPlayback::startHook()
	{
		return true;
	}

	void  ControlsPlayback::updateHook()
	{
		if(sequencing){
			controlOutput[0] = controlOutput[1] = Sequence[0][i_sequence];
			controlOutput[2] = Sequence[1][i_sequence];
			_controlOutputPort.write(controlOutput);
			i_sequence++;
			i_sequence = i_sequence % N_sequence;
		}
	}

	void  ControlsPlayback::stopHook()
	{
	}

	void  ControlsPlayback::cleanUpHook()
	{
	}

	bool ControlsPlayback::startSequence(){
		if(!loadSequence()){
			return false;
		}
		sequencing = true;
		i_sequence = 0;
		N_sequence = Sequence[0].size();
		return true;
	}

	bool ControlsPlayback::stopSequence(){
		sequencing = false;
		controlOutput[0] = controlOutput[1] = controlOutput[2] = 0.0;
		_controlOutputPort.write(controlOutput);
		return true;
	}

	bool  ControlsPlayback::loadSequence()
	{
		vector<vector<double> > S;
		loadMatrixFromDat(SEQUENCE_FILENAME, S);
		if(S.size()!=2){
			cout << "Sequence read from " << SEQUENCE_FILENAME << " does not have the correct size. It should have 2 rows. Maybe you have 2 columns?" << endl;
			return false;
		}
		else{
			Sequence = S;
			return true;
		}
	}

	void ControlsPlayback::loadMatrixFromDat(const char* filename, vector<vector<double> > &V)
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

