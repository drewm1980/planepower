#ifndef __MEASUREMENTSRECODER__
#define __MEASUREMENTSRECODER__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <ocl/OCL.hpp>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>

using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

#include "measurements_file_format.hpp"

namespace OCL
{
	class MeasurementsRecorder
		: public TaskContext
	{
	protected:

		Sample s;
		Sample * sampleSet;
		uint64_t samplesRecorded;

		/// 12 Markers + 12 cov. diag. matrix elements
		InputPort< vector< double > > portMeasurementsMarkers;
		vector< double > measurementsMarkers;
		FlowStatus statusMeasurementsMarkers;

		/// Measurements: x, y z, e11, ...,e33 + covariance diag elements matrix
		InputPort< vector< double > > portMeasurementsPose;
		vector< double > measurementsPose;
		FlowStatus statusMeasurementsPose;

		/// Measurements: wx, wy, wz, ax, ay, az + covariance matrix
		InputPort< vector< double > > portMeasurementsIMU;
		vector< double > measurementsIMU;
		FlowStatus statusMeasurementsIMU;

		/// Measurements: delta, ddelta
		InputPort< vector< double > > portMeasurementsEncoder;
		vector< double > measurementsEncoder;
		FlowStatus statusMeasurementsEncoder;

		/// Measurements: ur, up
		InputPort< vector< double > > portMeasurementsCtrl;
		vector< double > measurementsCtrl;
		FlowStatus statusMeasurementsCtrl;

	public:
		MeasurementsRecorder(std::string name);
		~MeasurementsRecorder();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
		
	};
}
#endif // __MEASUREMENTSRECODER__
