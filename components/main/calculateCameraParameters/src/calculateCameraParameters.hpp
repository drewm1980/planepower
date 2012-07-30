#ifndef __CALCULATECAMERAPARAMETERS__
#define __CALCULATECAMERAPARAMETERS__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/marsh/PropertyLoader.hpp>
#include <rtt/marsh/Marshalling.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <rtt/os/TimeService.hpp>

#include <ocl/OCL.hpp>

#include <opencv/cv.h>

#include <kdl/frames.hpp>
#include <kdl/frames_io.hpp>

#include <wrappers/matrix/matrix_wrapper.h>
#include <wrappers/matrix/vector_wrapper.h>
#include <pdf/gaussian.h>

#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;

namespace OCL
{

    /**
     */
    class CalculateCameraParameters
        : public TaskContext
    {
    protected:
        std::string			measurements_file;
        std::string			rotFile;
        std::string			transFile;
        std::string			projectionMatrixFile;
        Matrix				projectionMatrix;
        Frame				cameraWorldPose;

	double				X;
	double				Y;
	double				Z;
	double				Roll;
	double				Pitch;
	double				Yaw;

	double				P11;
	double				P12;
	double				P13;
	double				P21;
	double				P22;
	double				P23;
	double				P31;
	double				P32;
	double				P33;


    private:
        ifstream                    _measurementStream;
        RTT::os::TimeService::ticks     time_begin;
        double                      time_passed;

        CvMat*      _rot;
        CvMat*      _trans;
        CvMat*      projectionMatrixCv;
        int         _numImages;
        double      _xCamera, _yCamera, _zCamera, _rollCamera, _pitchCamera, _yawCamera;
        Frame       _sheetWorldPose;
        Frame       _cameraSheetPose;
        Frame       _sheetCameraPose;
        Frame       _cameraWorldPose;

        void calculateCameraParameters();
        void readMeasurement();

    public:
        CalculateCameraParameters(std::string name);
        ~CalculateCameraParameters();
        
        virtual bool configureHook();
        virtual bool startHook();
        
    };
}
#endif // __CALCULATECAMERAPARAMETERS__
