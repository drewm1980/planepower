#include "calculateCameraParameters.hpp"

#include <ocl/Component.hpp>
#include <rtt/marsh/Marshalling.hpp>

ORO_CREATE_COMPONENT( OCL::CalculateCameraParameters)

using namespace std;
using namespace RTT;
using namespace Orocos;

namespace OCL
{
     CalculateCameraParameters::CalculateCameraParameters(std::string name)
         : TaskContext(name,PreOperational)
     {
	log(Debug) << "(CalculateCameraParameters) constructor entered" << endlog();

	// Add properties

        // Now add it to the interface:
        addProperty("measurements_file",measurements_file).doc("text file containing the pose measurements of the calibration sheet");
        addProperty("rotFile",rotFile).doc("CV Mat xml file containing the camera extrinsics rotation");
        addProperty("transFile",transFile).doc("CV Mat xml file containing the camera extrinsics translations");
        addProperty("projectionMatrixFile",projectionMatrixFile).doc("CV Mat xml file containing the camera projection matrix");

        addProperty("X",X).doc("X Value");
        addProperty("Y",Y).doc("Y Value");
        addProperty("Z",Z).doc("Z Value");

        addProperty("Roll",Roll).doc("First rotate around X with R(oll) in radians");
        addProperty("Pitch",Pitch).doc("Next rotate around old Y with P(itch) in radians");
        addProperty("Yaw",Yaw).doc("Next rotate around old Z with Y(aw) in radians");

        addProperty("P11",P11).doc("1th element of vector");
        addProperty("P12",P12).doc("2th element of vector");
        addProperty("P13",P13).doc("3th element of vector");

        addProperty("P21",P21).doc("1th element of vector");
        addProperty("P22",P22).doc("2th element of vector");
        addProperty("P23",P23).doc("3th element of vector");

        addProperty("P31",P31).doc("1th element of vector");
        addProperty("P32",P32).doc("2th element of vector");
        addProperty("P33",P33).doc("3th element of vector");

	// Add operations

        addOperation("calculateCameraParameters", &CalculateCameraParameters::calculateCameraParameters,this).doc("Read a measurement from the file and put it on the ports");

	log(Debug) << "(CalculateCameraParameters) constructor finished" << endlog();
     }

    CalculateCameraParameters::~CalculateCameraParameters()
    {
        _measurementStream.close();
    }

    bool CalculateCameraParameters::configureHook()
    {
	log(Debug) << "(CalculateCameraParameters) configureHook entered" << endlog();
        bool result = true;

        _measurementStream.open( (measurements_file).c_str() ); // opens the file
        if(!_measurementStream)  // file couldn't be opened
        {
cout << "file: " << (measurements_file).c_str() << endl;
            log(Error) << "Error: file could not be opened" << endlog();
            log(Debug) << "(CalculateCameraParameters) finished ConfigureHook() " << endlog();
            result = false;
            return result;
        }   

	log(Debug) << "(CalculateCameraParameters) configureHook finished" << endlog();
        return result;
    }
    bool CalculateCameraParameters::startHook()
    {
	log(Debug) << "(CalculateCameraParameters) startHook entered" << endlog();
        bool result = true;
        time_begin = os::TimeService::Instance()->getTicks();
	log(Debug) << "(CalculateCameraParameters) configureHook finished" << endlog();
        return result;
    }
    void CalculateCameraParameters::calculateCameraParameters()
    {
        // get camera projection matrix from file
        projectionMatrixCv =  (CvMat*) cvLoad(projectionMatrixFile.c_str());
        projectionMatrix.resize(projectionMatrixCv->rows,projectionMatrixCv->cols);
        for (int row=0; row<projectionMatrixCv->rows ; row++)
        {
            for (int column=0; column<projectionMatrixCv->cols ; column++)
            {
                log(Debug) << "row " << row << endlog();
                log(Debug) << "column " << column << endlog();
                projectionMatrix(row+1,column+1) = CV_MAT_ELEM(*projectionMatrixCv,double,row,column);
                log(Debug) << "element " << CV_MAT_ELEM(*projectionMatrixCv,double,row,column) <<endlog();
                log(Debug) << "projectionMatrix(row+1,column+1) " << projectionMatrix(row+1,column+1) <<endlog();
            }
        }
	P11 = projectionMatrix(1,1);
	P12 = projectionMatrix(1,2);
	P13 = projectionMatrix(1,3);
	P21 = projectionMatrix(2,1);
	P22 = projectionMatrix(2,2);
	P23 = projectionMatrix(2,3);
	P31 = projectionMatrix(3,1);
	P32 = projectionMatrix(3,2);
	P33 = projectionMatrix(3,3);
        log(Debug) << "property set " << endlog();

        // get extrinsics these contain pose of sheet wrt cameras for different
        // calibration snapshots
        _rot = (CvMat*) cvLoad(rotFile.c_str());
        _trans = (CvMat*) cvLoad(transFile.c_str());

        // get number of images from extrinsics
        _numImages = _rot->rows ;
        log(Debug) << "number of calibration images " << _numImages << endlog();

        double roll;
        double pitch; 
        double yaw;
        double xAcc, yAcc, zAcc, rollAcc, yawAcc, pitchAcc;
        xAcc = 0.0;
        yAcc = 0.0;
        zAcc = 0.0;
        rollAcc = 0.0;
        pitchAcc = 0.0;
        yawAcc = 0.0;
        
        for(int imageCounter = 0 ; imageCounter < _numImages ; imageCounter++)
        {
            	log(Debug) << "calibration image " << imageCounter << endlog();
            	// get pose of camera with respect to sheet

            	// get rotation vector: x y z (size = angle rotated; unit vector
            	// = axis around which rotated)
            	Vector rotationVector = Vector(CV_MAT_ELEM(*_rot,float,imageCounter,0),CV_MAT_ELEM(*_rot,float,imageCounter,1),CV_MAT_ELEM(*_rot,float,imageCounter,2));
	
            	// create KDL rotation matrix from OpenCv rotation vector
            	_sheetCameraPose.M = Rotation::Rot(rotationVector,rotationVector.Norm());
            	_sheetCameraPose.p = Vector(CV_MAT_ELEM(*_trans,float,imageCounter,0) , CV_MAT_ELEM(*_trans,float,imageCounter,1) , CV_MAT_ELEM(*_trans,float,imageCounter,2) );
            	log(Debug) << "_sheetCameraPose " << _sheetCameraPose << endlog();

            	_cameraSheetPose = _sheetCameraPose.Inverse();
            	log(Debug) << "_cameraSheetPose " << _cameraSheetPose << endlog();
            	
            	// get pose of calibration sheet from file 
            	readMeasurement(); // fills in _sheetWorldPose
            	log(Debug) << "_sheetWorldPose " << _sheetWorldPose << endlog();

            	// calculate cameraPosition wrt sheet
            	cameraWorldPose=  _sheetWorldPose*_cameraSheetPose;
            	log(Debug) << "cameraWorldPose " << cameraWorldPose << endlog();

            	// sum of pose parameters to average
            	xAcc = xAcc + cameraWorldPose.p.x();
            	yAcc = yAcc + cameraWorldPose.p.y();
            	zAcc = zAcc + cameraWorldPose.p.z();

            	cameraWorldPose.M.GetRPY(roll,pitch,yaw);
            	rollAcc = rollAcc + roll;
            	pitchAcc = pitchAcc + pitch;
            	yawAcc = yawAcc + yaw;
        }
        // finish average pose of camera wrt world
        _xCamera = xAcc / ((double)_numImages);
        _yCamera = yAcc / ((double)_numImages);
        _zCamera = zAcc / ((double)_numImages);
        _rollCamera = rollAcc / ((double)_numImages);
        _pitchCamera = pitchAcc / ((double)_numImages);
        _yawCamera = yawAcc / ((double)_numImages);
        cameraWorldPose.p.x(_xCamera); 
        cameraWorldPose.p.y(_yCamera); 
        cameraWorldPose.p.z(_zCamera); 
        cameraWorldPose.M= Rotation::RPY(_rollCamera,_pitchCamera,_yawCamera); 
	X = _xCamera;
	Y = _yCamera;
	Z = _zCamera;
	Roll = _rollCamera;
	Pitch = _pitchCamera;
	Yaw = _yawCamera;
	this->getProvider<RTT::Marshalling>("marshalling")->writeProperties(this->getName()+".cpf");
        log(Debug) << "averaged cameraWorldPose " << cameraWorldPose << endlog();

    }

    void CalculateCameraParameters::readMeasurement()
    {
        // Fills in _sheetWorldPose
        time_begin = os::TimeService::Instance()->getTicks();
        log(Debug) << "***************************************" << endlog();
        log(Debug) << "readMeasurement entered" << endlog();

        if(!_measurementStream)  // file couldn't be opened
        {
            log(Error) << "Error: file could not be opened" << endlog();
        }   
        double num, x , y ,z, exx,exy,exz,eyx,eyy,eyz,ezx,ezy,ezz;
        _measurementStream >> num; // first one is time stamp
        _measurementStream >> x;
        _measurementStream >> y;
        _measurementStream >> z;
        _measurementStream >> exx;
        _measurementStream >> exy;
        _measurementStream >> exz;
        _measurementStream >> eyx;
        _measurementStream >> eyy;
        _measurementStream >> eyz;
        _measurementStream >> ezx;
        _measurementStream >> ezy;
        _measurementStream >> ezz;
        _sheetWorldPose.p.x(x);
        _sheetWorldPose.p.y(y);
        _sheetWorldPose.p.z(z);
	_sheetWorldPose.M=Rotation(exx,eyx,ezx,exy,eyy,ezy,exz,eyz,ezz);
cout << "Sheetworldpose: " << _sheetWorldPose << endl;
cout << endl << endl;
cout << "x: " << _sheetWorldPose.M.UnitX() << endl;

        log(Debug) << "_sheetWorldPose " << _sheetWorldPose << endlog();

        time_passed = os::TimeService::Instance()->secondsSince(time_begin);
        log(Debug) << "Time passed " << time_passed <<  endlog();
        log(Debug) << "readMeasurement finished" << endlog();
        log(Debug) << "***************************************" << endlog();
    }
}//namespace

