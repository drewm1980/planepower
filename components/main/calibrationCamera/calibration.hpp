#ifndef __CALIBRATOR__
#define __CALIBRATOR__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <rtt/Activity.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/os/TimeService.hpp>
//#include <rtt/RealTimeToolkit.hpp>

#include <ocl/OCL.hpp>
#include <ocl/FileReporting.hpp>

#include <kdl/frames.hpp>

//Everything for the OpenCV application specific
//#include <opencv/cv.h>
//#include <opencv/cvaux.h>
//#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <iostream>

//#include <ros/camera_calibration/calibrate.h>
//#include "/home/common/robotics/ros/pkgs/ros-pkg-trunk/stacks/image_pipeline/camera_calibration/include/camera_calibration/calibrate.h"
#include "calibrate.h"
//#include "camera_calibration/pinhole.h"


using std::ifstream;

using namespace std;
using namespace RTT;
using namespace KDL;
using namespace Orocos;
using namespace camera_calibration;

namespace OCL
{

    /// Calibrator class
    /**
    A class to calibrate a camera with a chessboard pattern. It needs
    boardWidth, boardHeight, squareSize, detectorSearchRadius and
    imageDirectory, searches for chessboard corners in the images, calibrates
    the camera and stores the cameraparameters in out.ini. 
     */
    class Calibrator
        : public TaskContext
    {

    private:
	    /// The chessboard detector	
		CheckerboardDetector* 	_detector;
        /// The camera calibrator
		Calibrater*		        _cal;
		/// Vector containing the different chessboard corners found in the image
		vector<CvPoint2D32f>  	_corners;
        /// Vector containing the different image files
        vector<string>          _imageFiles;
        /// The actual calibration method which searches for chessboard corners, calibrates the camera and saves the camera parameters
        void                    batchCalibrate();
        /// Method to get the different image files by reading the image directory
        vector<string>          getDir(string dir);
    
        vector<Frame>           _extrinsics;
	    /** Matrix pointer in which the extrinsics will be stored col 0-3 are
	        the rotation, col 3-6 are translation part, rows are for each image
        */ 	
        //CvMat*                  _extrinsics;
        CvMat*                  _rot;
        CvMat*                  _trans;

    protected:
        /*********
        PROPERTIES
        *********/
        /// The board width
        double				boardWidth;
        /// The board height
        double				boardHeight;
        /// The size of the squares
        double				squareSize;
        /// The radius in which the detector searches for corners
        double				detectorSearchRadius;
        /// The directory containing the chessboard images
        string				imageDirectory;
        /**********
        PORTS
        **********/
        /// Exports the camera extrinsics
        OutputPort<std::vector<Frame> >  _extrinsicPort;

    public:
	Calibrator(std::string name);
	~Calibrator();
        bool        	configureHook();
        bool        	startHook();
        void        	updateHook();
        void        	stopHook();
        void        	cleanUpHook();
        
    };
}
#endif // __CALIBRATOR__
