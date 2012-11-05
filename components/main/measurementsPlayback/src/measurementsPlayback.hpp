#ifndef __MEASUREMENTSPLAYBACK__
#define __MEASUREMENTSPLAYBACK__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>

#include <ocl/OCL.hpp>

#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

#include "measurements_file_format.hpp"

namespace OCL
{
	class MeasurementsPlayback
		: public TaskContext
	{
	protected:

		Sample s;
		Sample * sampleSet;
		uint64_t samplesRecorded;
		uint64_t samplesPlayed;
		
		/// 12 Markers + 12 cov. diag. matrix elements
		OutputPort< vector< double > > portMeasurementsMarkers;
		vector< double > measurementsMarkers;

		/// Measurements: x, y z, e11, ...,e33 + covariance diag elements matrix
		OutputPort< vector< double > > portMeasurementsPose;
		vector< double > measurementsPose;

		/// Measurements: wx, wy, wz, ax, ay, az + covariance matrix
		OutputPort< vector< double > > portMeasurementsIMU;
		vector< double > measurementsIMU;

		/// Measurements: delta, ddelta
		OutputPort< vector< double > > portMeasurementsEncoder;
		vector< double > measurementsEncoder;

		/// Measurements: ur, up
		OutputPort< vector< double > > portMeasurementsCtrl;
		vector< double > measurementsCtrl;

		/// Measurements: ur, up
		OutputPort< vector< double > > portMeasurementsCtrlRates;
		vector< double > measurementsCtrlRates;

	private:
	public:
		MeasurementsPlayback(std::string name);
		~MeasurementsPlayback();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
	};
}
#endif // __MEASUREMENTSPLAYBACK__
