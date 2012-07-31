#include "calibration.hpp"

#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::Calibrator )

using namespace std;
using namespace RTT;
using namespace KDL;
using namespace Orocos;
using namespace OCL;

namespace OCL
{
     Calibrator::Calibrator(std::string name)
         : TaskContext(name,PreOperational)
     {
	log(Debug) << "(Calibrator) constructor entered" << endlog();

        // Add properties

        addProperty("boardWidth",boardWidth).doc("number of internal corners in width");
        addProperty("boardHeight",boardHeight).doc("number of internal corners in heigth");
        addProperty("squareSize",squareSize).doc("size of square of the chessboard pattern");
        addProperty("detectorSearchRadius",detectorSearchRadius).doc("radius for checkboard detector");
        addProperty("imageDirectory",imageDirectory).doc("directory containing images for calibration");

	// Add ports

        addPort( "extrinsicCameraParameters",_extrinsicPort );

	// Add operations

        addOperation("batchCalibrate", &Calibrator::batchCalibrate, this).doc("Calibrate with batch images");

	log(Debug) << "(Calibrator) constructor finished" << endlog();
    }

    Calibrator::~Calibrator()
    {
    }

    bool Calibrator::configureHook()
    {
	log(Debug) << "(Calibrator) configureHook entered" << endlog();

        bool result = true;

        // create checkboard detector
        log(Debug) << "Create checkerboard detector " << endlog();
	    _detector = new CheckerboardDetector(boardWidth,boardHeight,squareSize);
  	    _detector->setFlags(CV_CALIB_CB_ADAPTIVE_THRESH);
  	    _detector->setSearchRadius(detectorSearchRadius);

        //_corners = vector<CvPoint2D32f>(_detector->corners() );

	    _corners.resize(_detector->corners());

        // Create calibrator
        log(Debug) << "Create calibrator " << endlog();
	    _cal = new Calibrater();
	    _cal->setFlags(CV_CALIB_FIX_PRINCIPAL_POINT);

  	    cvNamedWindow("Calibration", 0);

        // Get files from directory of images
        _imageFiles = getDir(imageDirectory);
        // TODO: check this construction
        if (!result)
        {
            log(Error) << "failed to getDir " << imageDirectory << endlog();
        }
        log(Debug) << "_imageFiles.size()" << _imageFiles.size( )<< endlog();
        for (unsigned int i = 0;i < _imageFiles.size();i++) {
            log(Debug) << _imageFiles[i] << endlog();
        }
        // Allocate memory for the extrinsic matrix
        _rot = cvCreateMat(_imageFiles.size(), 3, CV_32FC1);
	    _trans = cvCreateMat(_imageFiles.size(), 3, CV_32FC1);
       
        // TODO: Write first time to _extrinsicPort here to allocate the port
 
	log(Debug) << "(Calibrator) configureHook finished" << endlog();
        return result;
    }

    bool Calibrator::startHook()
    {
	log(Debug) << "(Calibrator) startHook entered" << endlog();
		
	    bool result = true; 	

	log(Debug) << "(Calibrator) startHook finished" << endlog();
        return result;
    }

    void Calibrator::updateHook()
    {
        log(Debug) << "(Calibrator) updateHook entered" << endlog();

        log(Debug) << "(Calibrator) updateHook finished" << endlog();
    }

    void Calibrator::stopHook()
    {
        log(Debug) << "(Calibrator) stopHook entered" << endlog();
		
        log(Debug) << "(Calibrator) stopHook finished" << endlog();
    }

    void Calibrator::cleanUpHook()
    {
        log(Debug) << "(Calibrator) cleanupHook entered" << endlog();

        delete _cal;
        delete _detector;
        // Release the allocated memory for the matrix
        cvReleaseMat(&_rot);	
        cvReleaseMat(&_trans);	
        
        log(Debug) << "(Calibrator) cleanupHook finished" << endlog();
    }

    void Calibrator::batchCalibrate()
    {
        log(Debug) << "(Calibrator) batchCalibrate entered" << endlog();
		

        int image_width = 0;
        int image_height = 0;
        log(Debug) << "calibration with " << _imageFiles.size() << " images" << endlog();
        for (unsigned int i = 0; i < _imageFiles.size(); ++i) {

            log(Debug) << "load image " << _imageFiles[i].c_str() <<endlog();
            cv::WImageBuffer1_b image( cvLoadImage(_imageFiles[i].c_str(), CV_LOAD_IMAGE_GRAYSCALE) );
            
            image_width = image.Width();
            image_height = image.Height();

            log(Debug) << "working on image name: " << _imageFiles[i].c_str() << endlog();

            int ncorners = 0;
            bool success = _detector->findCorners(image.Ipl(), &_corners[0], &ncorners);
            if (success) {
                log(Debug) << "found corners " << endlog();
              _cal->addView(&_corners[0], _detector->objectPoints(), _corners.size());
            } else {
            }

            cv::WImageBuffer3_b display(image.Width(), image.Height());
            cvCvtColor(image.Ipl(), display.Ipl(), CV_GRAY2BGR);
            cvDrawChessboardCorners(display.Ipl(), cvSize(boardWidth, boardHeight),&_corners[0], ncorners, success);
            cvShowImage("Calibration", display.Ipl());
            cvWaitKey(0);
        }
        /// Calibrate the model
        _cal->calibrate(image_width, image_height);
        /// Save the intrinsic camera parameters to file
        string fileName(imageDirectory);
        fileName.append("Intrinsics.ini");
        _cal->model().save(fileName.c_str());


        fileName = imageDirectory;
        fileName.append("K_cv.xml");
        cvSave(fileName.c_str(),&_cal->model().K_cv() );
        log(Debug) << "intrisics saved to " << fileName << endlog();

        /// Copy the extrinsic camera parameters out of the object
        //cvCopy( _cal.extrinsics_ , _extrinsics);
        _cal->getExtrinsics(*_rot,*_trans);
        
        /// Save the extrinsic data as a test TODO: put it on a bufferport
        //cvSave("extrinsic.xml", &_extrinsics);

        log(Debug) << "before cvSave " << endlog();
        fileName = imageDirectory;
        fileName.append("rot.xml");
        cvSave(fileName.c_str(),_rot);;
        fileName = imageDirectory;
        fileName.append("trans.xml");
        cvSave(fileName.c_str(), _trans);
        log(Debug) << "after cvSave " << endlog();

        log(Debug) << "(Calibrator) batchCalibrate finished" << endlog();
    }

    vector<string> Calibrator::getDir(string dir)
    {
        log(Debug) << "(Calibrator) getDir entered" << endlog();

        unsigned char isFile =0x8;
        vector<string>  files;
        DIR *dp;
        struct dirent *dirp;
        if((dp  = opendir(dir.c_str())) == NULL) {
            log(Error) <<"Error(" << errno << ") opening " << dir << endlog(); 
            return vector<string>(0);
        }
    
        while ((dirp = readdir(dp)) != NULL) {
            log(Debug) << "add file from imageDir " << endlog();
            if(dirp->d_type == isFile)
                files.push_back(dir + "/" + string(dirp->d_name));
        }
        closedir(dp);
        log(Debug) << "files.size()" << files.size( )<< endlog();
	std::sort( files.begin(), files.end() );

        log(Debug) << "(Calibrator) getDir finished" << endlog();
        return files;
    }
 
}//namespace

