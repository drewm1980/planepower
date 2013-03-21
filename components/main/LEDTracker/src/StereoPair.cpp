#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <math.h>

#include "StereoPair.hpp"
#include "cout.hpp"

#define VERBOSE 0 // Turns on and off printing of timing information.

using namespace std;
using std::ifstream;

StereoPair::StereoPair(bool useExternalTrigger)
{
	COUT << "(StereoPair) constructor entered" << ENDL;
	_useExternalTrigger = useExternalTrigger;	
	dc1394_driver = dc1394_new();
	assert(dc1394_driver);

	err = dc1394_camera_enumerate(dc1394_driver, &camera_list);
	if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	if(camera_list->num<2)
		CERR << "There are less than two cameras attached!!" << ENDL;
	COUT << "Attached cameras:" << ENDL;
	for(unsigned int i=0; i<camera_list->num; i++)
	{
		COUT << std::hex << camera_list->ids[i].guid << std::dec << ENDL;
	}

	right_guid = 0xb09d0100a98977;
	left_guid =  0xb09d0100a98974;
	uint64_t guids[2] = {right_guid, left_guid};
	for(int i=0; i<2; i++)
	{
		cameras[i] = dc1394_camera_new(dc1394_driver,guids[i]); 
		if (!cameras[i]) {
			CERR << "Failed to initialize camera with guid " << guids[i] << ENDL;
			continue;
		}
	}
	COUT << "Successfully connected to both cameras..." << ENDL;

	for(unsigned int i=0; i < 2; i++) {
		err=dc1394_video_set_transmission(cameras[i], DC1394_OFF);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		err=dc1394_video_set_operation_mode(cameras[i], DC1394_OPERATION_MODE_1394B);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		err=dc1394_video_set_iso_speed(cameras[i], DC1394_ISO_SPEED_800);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

		err=dc1394_video_set_mode(cameras[i], DC1394_VIDEO_MODE_1600x1200_MONO8);
		frame_w=1600;
		frame_h=1200;
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
	for(unsigned int i=0; i < 2; i++) {
		if(BROADCAST) {
			err = dc1394_camera_set_broadcast(cameras[i], DC1394_TRUE);
		} else {
			err = dc1394_camera_set_broadcast(cameras[i], DC1394_FALSE);
		}
		if(err!=DC1394_SUCCESS) CERR << dc1394_error_get_string(err) << ENDL;
	}

	for(unsigned int i=0; i < 2; i++) {
		current_frame[i] = NULL;
	}

	transfer_time = frame_h*frame_w*8.0f / 800.0e6f;  // sec.  the bus ~should run at 800 Mbps
	COUT << "Based on bus speed, transfer time should be " << transfer_time*1e3 << "ms" << ENDL;
	period = 1.0 / f_fps; // sec

	// Read and Sanity check the camera exposure times
	float exposure_time = 999.0f;
	dc1394bool_t absolute_supported;
	err = dc1394_feature_has_absolute_control(cameras[0], DC1394_FEATURE_SHUTTER, &absolute_supported);
	if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_SHUTTER, &exposure_time);
	if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	for(unsigned int i=1; i<2; i++)
	{
		float temp_exposure_time = -1.0f;
		dc1394bool_t absolute_supported;
		err = dc1394_feature_has_absolute_control(cameras[i], 
				DC1394_FEATURE_SHUTTER, 
				&absolute_supported);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		err = dc1394_feature_get_absolute_value(cameras[0], 
				DC1394_FEATURE_SHUTTER, 
				&temp_exposure_time);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		if((temp_exposure_time - exposure_time) > 1e-9)
		{
			COUT << "Shutter values from the cameras do not match!\n"
				<< "This might screw up frame arrival time estimates!"
				<< "Please double-check the cameras using coriander or flycap!" << ENDL;
		}
	}

	COUT << "(StereoPair) constructor finished" << ENDL;
}

StereoPair::~StereoPair()
{
	free(camera_list);
	free(cameras);
	free(current_frame);
	free(last_frame_timestamps);
	free(current_frame_timestamps);
}

bool  StereoPair::startHook()
{
	COUT << "(StereoPair) startHook entered" << ENDL;

	if(BROADCAST)
	{
		dc1394_video_set_transmission(cameras[0], DC1394_ON);
	}
	else{
		for(unsigned int i=0; i < 2; i++) {
			dc1394_video_set_transmission(cameras[i], DC1394_ON);
		}
	}

	//Clear out the framebuffers of all cameras asap so that we know
	//we have the freshest frame from all cameras.
	for(unsigned int i=0; i < 2; i++) {
		flush_buffer(cameras[i]);
	}

	COUT << "(StereoPair) startHook finished" << ENDL;
	return true;
}

void StereoPair::flush_buffer(dc1394camera_t * cam)
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

void StereoPair::tweak_frame_arrival_times()
{
	for(unsigned int i=0; i < 2; i++) {
		last_frame_timestamps[i] = current_frame_timestamps[i];
		current_frame_timestamps[i] = current_frame[i] -> timestamp;
	}
	int64_t arrival_disparity = current_frame_timestamps[1] - current_frame_timestamps[0]; // us
#if VERBOSE
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

void  StereoPair::updateHook()
{
	//COUT << "(StereoPair) updateHook entered" << ENDL;
	for(int i=0; i < 2; i++) {
		//int frames_behind = 0;
		//do{
			// Enqueue the previously captured frame, if there is one.
			if(current_frame[i] != NULL)
			{
				err = dc1394_capture_enqueue(cameras[i], current_frame[i]);
				if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
				//COUT << "(StereoPair) enqueud a frame!" << ENDL;
			}
			
			//COUT << "(StereoPair) dequeueing a frame from camera " << i << ENDL;
			err = dc1394_capture_dequeue(cameras[i], DC1394_CAPTURE_POLICY_WAIT, &current_frame[i]);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

			//frames_behind = current_frame[i]->frames_behind;
			////COUT << "(StereoPair) Camera " << i+1 
						////<< ", frames_behind=" << frames_behind << ENDL;
			//if(frames_behind>0)
			//{
				//COUT << "(StereoPair) Camera " << i+1 << " is Dropping a frame!"<< ENDL;
			//}
		//}
		//while(frames_behind>0);
		current_frame_data[i] = current_frame[i]->image;
	}
	//COUT << "(StereoPair) done dequeuing" << ENDL;

	tweak_frame_arrival_times(); // us

	//COUT << "(StereoPair) updateHook finished" << ENDL;
}

void  StereoPair::stopHook()
{
	COUT << "(StereoPair) stopHook entered" << ENDL;
	for(unsigned int i=0; i < 2; i++) {
		dc1394_video_set_transmission(cameras[i], DC1394_OFF);
		dc1394_capture_stop(cameras[i]);
	}
	COUT << "(StereoPair) stopHook finished" << ENDL;
}

void  StereoPair::cleanUpHook()
{
	COUT << "(StereoPair) cleanupHook entered" << ENDL;
	for(unsigned int i=0; i < 2; i++) {
		err = dc1394_camera_set_broadcast(cameras[i], DC1394_FALSE);
		if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	}
	for(unsigned int i=0; i < 2; i++) {
		dc1394_camera_free(cameras[i]);
	}
	dc1394_camera_free_list(camera_list);
	dc1394_free(dc1394_driver);
	COUT << "(StereoPair) cleanupHook finished" << ENDL;
}
