#include "measurementsPlayback.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::MeasurementsPlayback)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
	MeasurementsPlayback::MeasurementsPlayback(std::string name) : TaskContext(name) { 
		addPort("portMeasurementsMarkers",portMeasurementsMarkers);
		addPort("portMeasurementsIMU",portMeasurementsIMU);
		addPort("portMeasurementsEncoder",portMeasurementsEncoder);
		addPort("portMeasurementsCtrl",portMeasurementsCtrl);
		addPort("portMeasurementsPose",portMeasurementsPose);

	}
	MeasurementsPlayback::~MeasurementsPlayback() { } 

	bool  MeasurementsPlayback::configureHook()
	{
		cout << "(measurementsRecorder) Loading from file..." << endl;
		ifstream myfile;
		myfile.open (DEFAULT_LOG_FILE_NAME, ios::in|ios::binary);
		if(myfile.fail())
		{
			cout << "(measurementsPlayback) Couldn't open file " 
				<< DEFAULT_LOG_FILE_NAME << endl;
		}
		myfile.read((char*)&samplesRecorded,sizeof(samplesRecorded));
		cout << "samplesRecorded read from file: " << samplesRecorded << endl;

		sampleSet = (Sample*) calloc(samplesRecorded,sizeof(Sample)); 
		if(sampleSet==NULL)
		{
			cout << "(measurementsPlayback) Couldn't malloc enough memory to load from file!" << endl;
			return false;
		}
		myfile.read((char*)sampleSet,samplesRecorded*sizeof(Sample));
		myfile.close();
		cout << "(measurementsPlayback) Done Loading from file!" << endl;

		measurementsMarkers.resize(NY_MARK*2);
		measurementsIMU.resize(IMU_VEC_SIZE);
		measurementsEncoder.resize(NY_ENC);
		measurementsCtrl.resize(NY_CTRL);
		measurementsPose.resize(NY_POSE);
		cout << "(measurementsPlayback) Done resizing port vectors!" << endl;

		return true;
	}

	bool  MeasurementsPlayback::startHook()
	{
		samplesPlayed=0;
		return true;
	}

	void  MeasurementsPlayback::updateHook()
	{
		if(samplesPlayed<samplesRecorded)
		{
			s = sampleSet[samplesPlayed]; // Copy instance of Sample
			
			// Copy our data from Sample s
			copy(s.measurementsMarkers,s.measurementsMarkers+NY_MARK*2,measurementsMarkers.begin());
			copy(s.measurementsIMU,s.measurementsIMU+IMU_VEC_SIZE,measurementsIMU.begin());
			copy(s.measurementsEncoder,s.measurementsEncoder+NY_ENC,measurementsEncoder.begin());
			copy(s.measurementsCtrl,s.measurementsCtrl+NY_CTRL,measurementsCtrl.begin());
			copy(s.measurementsPose,s.measurementsPose+NY_POSE,measurementsPose.begin());

			cout << "Done copying the s into the ports" << endl;
		
			// Write our ports.
			portMeasurementsMarkers.write( measurementsMarkers );
			portMeasurementsIMU.write( measurementsIMU );
			portMeasurementsEncoder.write( measurementsEncoder );
			portMeasurementsCtrl.write( measurementsCtrl );
			portMeasurementsPose.write( measurementsPose );
			
			cout << "Done writing the ports" << endl;

			samplesPlayed+=1;
		} else {
			cout << "(measurementsPlayback) Reached end of recorded data!" << endl;
			stop();
		}
	}

	void  MeasurementsPlayback::stopHook() { } 

	void  MeasurementsPlayback::cleanUpHook()
	{
		free(sampleSet);
	}
}//namespace

