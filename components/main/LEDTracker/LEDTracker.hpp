#ifndef __LEDTRACKER__
#define __LEDTRACKER__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <ocl/OCL.hpp>

#include "CameraArray.hpp"
#include "BlobExtractor.hpp"

#define MARKER_NOT_DETECTED_VALUE -1000.0

#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace Orocos;
typedef uint64_t TIME_TYPE;

namespace OCL
{
	/**
	 * This class captures images from a single firewire camera,
	 * and finds the location of three LED markers in the image.
	 * The emphasis is on speed, rather than robustness.
	  */
	class LEDTracker : public TaskContext
	{
		protected:
			OutputPort< vector<double> >	_markerPositions;
			// Warning!!! These are actually invers covariances, i.e. weights!!!
			OutputPort< vector<double> >	_markerPositionsAndCovariance;
			Attribute<bool> _useExternalTrigger;
			InputPort<TIME_TYPE>			_triggerTimeStampIn;
			TIME_TYPE				triggerTimeStamp;
			OutputPort<TIME_TYPE>			_triggerTimeStampOut;
			OutputPort<double>			_compTime;

			OutputPort<TIME_TYPE>			_frameArrivalTimeStamp;
			TIME_TYPE				frameArrivalTimeStamp;
			OutputPort<TIME_TYPE>			_computationCompleteTimeStamp;
			TIME_TYPE				computationCompleteTimeStamp;

			InputPort<double> _deltaIn;
			OutputPort<double> _deltaOut;
			double delta;

			int frame_w, frame_h;

		private:
			CameraArray *cameraArray;
			BlobExtractor *blobExtractors[CAMERA_COUNT];

			// These are staging areas for the ports
			vector<double>	markerPositions;
			vector<double>	markerPositionsAndCovariance;

			double			sigma_marker;
			TIME_TYPE		tempTime;
			
		public:
			LEDTracker(std::string name);
			~LEDTracker();
			bool        configureHook();
			bool        startHook();
			void        updateHook();
			void        stopHook();
			void        cleanUpHook();
	};
}
#endif // __LEDTRACKER__
