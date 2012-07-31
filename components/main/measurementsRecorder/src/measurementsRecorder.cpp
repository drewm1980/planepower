#include "measurementsRecoder.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::MeasurementsRecoder)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
	MeasurementsRecoder::MeasurementsRecoder(std::string name) : TaskContext(name) { }
	MeasurementsRecoder::~MeasurementsRecoder() { } 

	bool  MeasurementsRecoder::configureHook()
	{
		sampleSet = (Sample*) calloc(BUFFER_IN_SAMPLES,sizeof(Sample)); 
		if(sampleSet==NULL)
		{
			cout << "(measurementsPlayback) Couldn't malloc enough memory!" << endl;
			return false;
		}
		return true;
	}

	bool  MeasurementsRecoder::startHook() { 
		samplesRecorded=0;
		return true; 
	}
	void  MeasurementsRecoder::updateHook()
	{
		// Read our ports into stl vectors the traditional way.
		statusMeasurementsMarkers = portMeasurementsMarkers.read( measurementsMarkers );
		statusMeasurementsIMU = portMeasurementsIMU.read( measurementsIMU );
		statusMeasurementsEncoder = portMeasurementsEncoder.read( measurementsEncoder );
		statusMeasurementsCtrl = portMeasurementsCtrl.read( measurementsCtrl );
		statusMeasurementsPose = portMeasurementsPose.read( measurementsPose );
		
		// Copy our data into Sample s
		copy(measurementsMarkers.begin(),measurementsMarkers.end(),s.measurementsMarkers);
		copy(measurementsIMU.begin(),measurementsIMU.end(),s.measurementsIMU);
		copy(measurementsEncoder.begin(),measurementsEncoder.end(),s.measurementsEncoder);
		copy(measurementsCtrl.begin(),measurementsCtrl.end(),s.measurementsCtrl);
		copy(measurementsPose.begin(),measurementsPose.end(),s.measurementsPose);

		if(samplesRecorded<BUFFER_IN_SAMPLES)
		{
			sampleSet[samplesRecorded+1] = s;
			samplesRecorded+=1;
		} else {
			cout << "(measurementsRecorder) End of Buffer reached!" << endl;
			stop();
		}
	}

	void  MeasurementsRecoder::stopHook()
	{
		cout << "(measurementsRecorder) Stopping recording!" << endl;
		cout << "(measurementsRecorder) Dumping to file..." << endl;
		ofstream myfile;
		myfile.open (DEFAULT_LOG_FILE_NAME, ios::out|ios::binary|ios::trunc);
		myfile.write((char*)&samplesRecorded,sizeof(samplesRecorded));
		myfile.write((char*)sampleSet,samplesRecorded*sizeof(Sample));
		myfile.close();
		cout << "(measurementsRecorder) Done Dumping to file!" << endl;
	}

	void  MeasurementsRecoder::cleanUpHook() {
		free(sampleSet);
	} 
}//namespace

