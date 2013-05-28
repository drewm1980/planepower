#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>

#include "CameraArray.hpp"
#include "cout.hpp"

#define VERBOSE 0 // Turns on and off printing of timing information.

using namespace std;
using std::ifstream;

CameraArray::CameraArray(bool useExternalTrigger)
{
	COUT << "(CameraArray) constructor entered" << ENDL;
	_useExternalTrigger = useExternalTrigger;	
	dc1394_driver = dc1394_new();
	assert(dc1394_driver);

	err = dc1394_camera_enumerate(dc1394_driver, &camera_list);
	if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	if(camera_list->num<CAMERA_COUNT)
	{
		CERR << "There are less than the expected " << CAMERA_COUNT << " cameras attached!!" << ENDL;
	}
	COUT << "Attached cameras:" << ENDL;
	for(unsigned int i=0; i<camera_list->num; i++)
	{
		COUT << std::hex << camera_list->ids[i].guid << std::dec << ENDL;
	}

	const uint64_t guids[CAMERA_COUNT] = GUIDS;

	for(int i=0; i<CAMERA_COUNT; i++)
	{
		cameras[i] = dc1394_camera_new(dc1394_driver,guids[i]); 
		if (!cameras[i]) {
			CERR << "Failed to initialize camera with guid " << guids[i] << " index " << i << ENDL;
			continue;
		}
	}
	COUT << "Successfully connected to all cameras..." << ENDL;

	// Set up trigger mode on all cameras
	for(unsigned int i=0; i < CAMERA_COUNT; i++) 
	{
		err=dc1394_video_set_transmission(cameras[i], DC1394_OFF);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		err=dc1394_video_set_operation_mode(cameras[i], DC1394_OPERATION_MODE_1394B);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		err=dc1394_video_set_iso_speed(cameras[i], DC1394_ISO_SPEED_800);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		err=dc1394_video_set_mode(cameras[i], VIDEO_MODE);
		frame_w=FRAME_W;
		frame_h=FRAME_H;
		camera_count = CAMERA_COUNT;
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		if(_useExternalTrigger)
		{
			f_fps = nanf("External");
			err=dc1394_external_trigger_set_power(cameras[i], DC1394_ON);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			//err=dc1394_external_trigger_set_mode(cameras[i], DC1394_TRIGGER_MODE_0); // Didn't allow more than 7 Hz framerate!
			err=dc1394_external_trigger_set_mode(cameras[i], DC1394_TRIGGER_MODE_14); // Goes up to (almost 15 Hz)
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
#if TRIGGER_ACTIVE_HIGH
			err=dc1394_external_trigger_set_polarity(cameras[i], DC1394_TRIGGER_ACTIVE_HIGH);	
#else
			err=dc1394_external_trigger_set_polarity(cameras[i], DC1394_TRIGGER_ACTIVE_LOW);	
#endif
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		} else {
			err=dc1394_external_trigger_set_power(cameras[i], DC1394_OFF);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			f_fps = 15.0f;
			err=dc1394_video_set_framerate(cameras[i], DC1394_FRAMERATE_15); 
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		}

		err=dc1394_capture_setup(cameras[i], FRAME_BUFFER_DEPTH, DC1394_CAPTURE_FLAGS_DEFAULT);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

	}

	// Put all cameras into broadcast mode
	for(unsigned int i=0; i < CAMERA_COUNT; i++) {
		if(BROADCAST) {
			err = dc1394_camera_set_broadcast(cameras[i], DC1394_TRUE);
		} else {
			err = dc1394_camera_set_broadcast(cameras[i], DC1394_FALSE);
		}
		if(err!=DC1394_SUCCESS) CERR << dc1394_error_get_string(err) << ENDL;
	}

	for(unsigned int i=0; i < CAMERA_COUNT; i++) {
		current_frame[i] = NULL;
	}

	transfer_time = CAMERA_COUNT*frame_h*frame_w*8.0f / 800.0e6f;  // sec.  the bus ~should run at 800 Mbps
	if (VIDEO_MODE==DC1394_VIDEO_MODE_800x600_RGB8) transfer_time *= 3.0f;  // Three channels
	COUT << "Based on bus speed, transfer time should be " << transfer_time*1e3 << "ms" << ENDL;
	period = 1.0 / f_fps; // sec

	// Set all of the camera settings at startup.  
	// This will hopefully insulate against the cameras loosing their values.
	for(unsigned int i=0; i<CAMERA_COUNT; i++)
	{
		// Set shutter (exposure time)
		//exposure_time = .00003;
		exposure_time = .0001;
		err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_SHUTTER, exposure_time);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		//// Set up brightness
		//err = dc1394_feature_set_mode(cameras[i], DC1394_FEATURE_BRIGHTNESS, DC1394_FEATURE_MODE_MANUAL)
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		//// Set up Exposure
		//err = dc1394_feature_set_mode(cameras[i], DC1394_FEATURE_EXPOSURE, DC1394_FEATURE_MODE_MANUAL)
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	}

	// Print settings for first camera
#if VERBOSE
	dc1394_featureset_t features_for_printing;
	dc1394_feature_get_all(cameras[0], &features_for_printing);
	dc1394_feature_print_all(&feature_for_printing); // second parameter for stdout?
#endif

	COUT << "(CameraArray) constructor finished" << ENDL;
}

CameraArray::~CameraArray()
{
	free(camera_list);
	free(cameras);
	free(current_frame);
	free(last_frame_timestamps);
	free(current_frame_timestamps);
}

bool  CameraArray::startHook()
{
	COUT << "(CameraArray) startHook entered" << ENDL;

	if(BROADCAST)
	{
		dc1394_video_set_transmission(cameras[0], DC1394_ON);
	}
	else{
		for(unsigned int i=0; i < CAMERA_COUNT; i++) {
			dc1394_video_set_transmission(cameras[i], DC1394_ON);
		}
	}

	//Clear out the framebuffers of all cameras asap so that we know
	//we have the freshest frame from all cameras.
	for(unsigned int i=0; i < CAMERA_COUNT; i++) {
		flush_buffer(cameras[i]);
	}

	COUT << "(CameraArray) startHook finished" << ENDL;
	return true;
}

void CameraArray::flush_buffer(dc1394camera_t * cam)
{
	dc1394video_frame_t * f = NULL;
	while(true)
	{
		err = dc1394_capture_dequeue(cam, DC1394_CAPTURE_POLICY_POLL, &f);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		if(f!=NULL)
		{
			err = dc1394_capture_enqueue(cam, f);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		} else {
			break;
		}
	}
}

void CameraArray::tweak_frame_arrival_times()
{
	for(unsigned int i=0; i < CAMERA_COUNT; i++) {
		last_frame_timestamps[i] = current_frame_timestamps[i];
		current_frame_timestamps[i] = current_frame[i] -> timestamp;
	}
#if VERBOSE
	int64_t arrival_disparity = current_frame_timestamps[1] - current_frame_timestamps[0]; // us
	float f_arrival_disparity = (float)arrival_disparity*1.0e-6; // sec
	float measured_period = (double)(current_frame_timestamps[0]-last_frame_timestamps[0])*1e-6; // sec
	COUT 
		<< "dt=" << f_arrival_disparity/period << " periods" 
		<< " period=" << period << " sec"
		<< " f=" << 1.0f/measured_period << " Hz" 
		<< ENDL;
#endif
	uint64_t arrival_timestamp = max(current_frame_timestamps[0], current_frame_timestamps[1]);
	current_frame_timestamp_tweaked = (uint64_t)(arrival_timestamp - transfer_time*1e6 - exposure_time/2.0f*1e6);
}

void  CameraArray::updateHook()
{
	//COUT << "(CameraArray) updateHook entered" << ENDL;
	for(int i=0; i < CAMERA_COUNT; i++) {
		//int frames_behind = 0;
		//do{
			// Enqueue the previously captured frame, if there is one.
			if(current_frame[i] != NULL)
			{
				err = dc1394_capture_enqueue(cameras[i], current_frame[i]);
				if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
				//COUT << "(CameraArray) enqueud a frame!" << ENDL;
			}
			
			//COUT << "(CameraArray) dequeueing a frame from camera " << i << ENDL;
			err = dc1394_capture_dequeue(cameras[i], DC1394_CAPTURE_POLICY_WAIT, &current_frame[i]);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

			//frames_behind = current_frame[i]->frames_behind;
			////COUT << "(CameraArray) Camera " << i+1 
						////<< ", frames_behind=" << frames_behind << ENDL;
			//if(frames_behind>0)
			//{
				//COUT << "(CameraArray) Camera " << i+1 << " is Dropping a frame!"<< ENDL;
			//}
		//}
		//while(frames_behind>0);
		current_frame_data[i] = current_frame[i]->image;
	}
	//COUT << "(CameraArray) done dequeuing" << ENDL;

	tweak_frame_arrival_times(); // us

	//COUT << "(CameraArray) updateHook finished" << ENDL;
}

void  CameraArray::stopHook()
{
	COUT << "(CameraArray) stopHook entered" << ENDL;
	for(unsigned int i=0; i < CAMERA_COUNT; i++) {
		dc1394_video_set_transmission(cameras[i], DC1394_OFF);
		dc1394_capture_stop(cameras[i]);
	}
	COUT << "(CameraArray) stopHook finished" << ENDL;
}

void  CameraArray::cleanUpHook()
{
	COUT << "(CameraArray) cleanupHook entered" << ENDL;
	for(unsigned int i=0; i < CAMERA_COUNT; i++) {
		err = dc1394_camera_set_broadcast(cameras[i], DC1394_FALSE);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	}
	for(unsigned int i=0; i < CAMERA_COUNT; i++) {
		dc1394_camera_free(cameras[i]);
	}
	dc1394_camera_free_list(camera_list);
	dc1394_free(dc1394_driver);
	COUT << "(CameraArray) cleanupHook finished" << ENDL;
}
