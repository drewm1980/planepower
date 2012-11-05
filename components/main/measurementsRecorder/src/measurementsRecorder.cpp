#include "measurementsRecorder.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::MeasurementsRecorder)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
	MeasurementsRecorder::MeasurementsRecorder(std::string name) : TaskContext(name) {
       
	addPort("portMeasurementsMarkers",portMeasurementsMarkers);
	addPort("portMeasurementsIMU",portMeasurementsIMU);
	addEventPort("portMeasurementsEncoder",portMeasurementsEncoder);
	addPort("portMeasurementsCtrl",portMeasurementsCtrl);
	addPort("portMeasurementsCtrlRates",portMeasurementsCtrlRates);
	addPort("portMeasurementsPose",portMeasurementsPose);


	}
	MeasurementsRecorder::~MeasurementsRecorder() { } 

	bool  MeasurementsRecorder::configureHook()
	{
		sampleSet = (Sample*) calloc(BUFFER_IN_SAMPLES,sizeof(Sample)); 
		if(sampleSet==NULL)
		{
			cout << "(measurementsPlayback) Couldn't malloc enough memory!" << endl;
			return false;
		}
		return true;
	}

	bool  MeasurementsRecorder::startHook() { 
		samplesRecorded=0;
		return true; 
	}
	void  MeasurementsRecorder::updateHook()
	{
		// Read our ports into stl vectors the traditional way.
		statusMeasurementsMarkers = portMeasurementsMarkers.read( measurementsMarkers );
		statusMeasurementsIMU = portMeasurementsIMU.read( measurementsIMU );
		statusMeasurementsEncoder = portMeasurementsEncoder.read( measurementsEncoder );
		statusMeasurementsCtrl = portMeasurementsCtrl.read( measurementsCtrl );
		statusMeasurementsCtrlRates = portMeasurementsCtrlRates.read( measurementsCtrlRates );
		statusMeasurementsPose = portMeasurementsPose.read( measurementsPose );
		
		// Copy our data into Sample s
		copy(measurementsMarkers.begin(),measurementsMarkers.end(),s.measurementsMarkers);
		copy(measurementsIMU.begin(),measurementsIMU.end(),s.measurementsIMU);
		copy(measurementsEncoder.begin(),measurementsEncoder.end(),s.measurementsEncoder);
		copy(measurementsCtrl.begin(),measurementsCtrl.end(),s.measurementsCtrl);
		copy(measurementsCtrlRates.begin(),measurementsCtrlRates.end(),s.measurementsCtrlRates);
		copy(measurementsPose.begin(),measurementsPose.end(),s.measurementsPose);

		if(samplesRecorded<BUFFER_IN_SAMPLES)
		{
			sampleSet[samplesRecorded] = s;
			samplesRecorded+=1;
		} else {
			cout << "(measurementsRecorder) End of Buffer reached!" << endl;
			stop();
		}
	}

	void  MeasurementsRecorder::stopHook()
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

	void  MeasurementsRecorder::cleanUpHook() {
		free(sampleSet);
	} 
}//namespace

