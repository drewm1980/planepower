#include "poseFromMarkers.hpp"
#include <ocl/Component.hpp>
#include <math.h>
#include <string.h>

#include <opencv2/opencv.hpp>

ORO_CREATE_COMPONENT( OCL::PoseFromMarkers)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;
using namespace cv;

#include "pose_from_markers.h"

namespace OCL
{
	 PoseFromMarkers::PoseFromMarkers(std::string name)
		 : TaskContext(name)
	 {
	// Add ports

		addEventPort( "markerPositions",_markerPositions, boost::bind(&PoseFromMarkers::getPose,this) ).doc("The positions of the markers in the camera");
		addPort( "pose",_pfm ).doc("The body pose w.r.t. the anchorpoint frame, based on the marker positions in the cameras: x,y,z,e11,e12,e13,e21,e22,e23,e31,e32,e33");
		//addPort( "cov_rt_inv",_cov_rt_inv ).doc("The estimated (state dependent) covariance matrix of the pose measurement vector.");
		//addPort( "cov_rt",_cov_rt ).doc("covariance of pose estimate");
		addPort( "pose_and_cov_rt_inv",_pose_and_cov_rt_inv ).doc("The estimated (state dependent) covariance matrix of the pose measurement vector.");

	markerPositions.resize(NMARKERPOSITIONS,0.0);
	pfm.resize(NPOSE,0.0);
	cov_rt.resize(NPOSE*NPOSE,0.0);
	cov_rt_inv.resize(NPOSE*NPOSE,0.0);
	pose_and_cov_rt_inv.resize(NPOSE + NPOSE*NPOSE,0.0);
	
	getPoseWrapper_input[0] = getPoseWrapper_markerPositions;
	getPoseWrapper_output[0] = getPoseWrapper_pose;
	getPoseWrapper_output[1] = getPoseWrapper_cov_rt;
	getPoseWrapper_output[2] = getPoseWrapper_cov_rt_inv;

        _pfm.setDataSample( pfm );
        _pfm.write( pfm );
        _pose_and_cov_rt_inv.setDataSample( pose_and_cov_rt_inv );
        _pose_and_cov_rt_inv.write( pose_and_cov_rt_inv );
	}

	PoseFromMarkers::~PoseFromMarkers()
	{
	}

	bool  PoseFromMarkers::configureHook()
	{
		return true;
	 }

	bool  PoseFromMarkers::startHook()
	{
		return true;
	}

	void  PoseFromMarkers::updateHook()
	{
	}

	void  PoseFromMarkers::stopHook()
	{
	}

	void  PoseFromMarkers::cleanUpHook()
	{
	}

	void PoseFromMarkers::getPose()
	{
		_markerPositions.read(markerPositions);
		int found_nan = 0;
		for(int i=0; i<NMARKERPOSITIONS; i++)
		{
			if(isnan(markerPositions[i]))
				found_nan = 1;
		}
		if(found_nan)
		{
			//memset(pfm,0,NPOSE*sizeof(double)):
			//memset(cov_rt_inv,0,NPOSE*NPOSE*sizeof(double));
			memset(&(pose_and_cov_rt_inv[0]), 0, (NPOSE+NPOSE*NPOSE) * sizeof(double) );
		} 
		else 
		{
			getPoseWrapper(markerPositions,&pfm,&cov_rt,&cov_rt_inv);

			memcpy(&pose_and_cov_rt_inv[0],&pfm[0],NPOSE*sizeof(double)); // Copy the pose part
#define NUMERIC_INVERSE 1
#if NUMERIC_INVERSE
			// Our Jacobian is actually singular, so we need to be careful about inversion
			//
			//
			CvMat* cov_rt_opencv = cvCreateMatHeader(NPOSE,NPOSE,CV_64FC1);
			cov_rt_opencv->data.db = &(cov_rt[0]);	
			CvMat* cov_rt_inv_opencv = cvCreateMatHeader(NPOSE,NPOSE,CV_64FC1);
			cov_rt_inv_opencv->data.db = &(pose_and_cov_rt_inv[NPOSE]);	
			//Mat cov_rt_opencv(NPOSE,NPOSE,CV_64FC1,&(cov_rt[0]));
			//Mat cov_rt_inv_opencv(NPOSE,NPOSE,CV_64FC1);
			
			cvInvert(cov_rt_opencv, cov_rt_inv_opencv, CV_SVD);
			//memcpy(&pose_and_cov_rt_inv[NPOSE],cov_rt_inv_opencv.data,NPOSE*NPOSE*sizeof(double));
#else
			memcpy(&pose_and_cov_rt_inv[NPOSE],&cov_rt_inv[0],NPOSE*NPOSE*sizeof(double)); // Copy the covariance part
#endif
		}
		_pfm.write(pfm);
		//_cov_rt.write(cov_rt);
		//_cov_rt_inv.write(cov_rt_inv);
		_pose_and_cov_rt_inv.write(pose_and_cov_rt_inv);
	}

	int PoseFromMarkers::getPoseWrapper(vector<double> markerPositions
										, vector<double> *pose
										, vector<double> *cov_rt
										, vector<double> *cov_rt_inv)
	{
		copy(markerPositions.begin(), markerPositions.end(), getPoseWrapper_input[0]);

		if( pose_from_markers(getPoseWrapper_input, getPoseWrapper_output) ){ return 1;}

		(*pose).assign(getPoseWrapper_output[0]
				 	  ,getPoseWrapper_output[0]+NPOSE);
		(*cov_rt).assign(getPoseWrapper_output[1]
				 	  ,getPoseWrapper_output[1]+NPOSE*NPOSE);
		(*cov_rt_inv).assign(getPoseWrapper_output[2]
				 	  ,getPoseWrapper_output[2]+NPOSE*NPOSE);
		return 0;
	}


}//namespace

