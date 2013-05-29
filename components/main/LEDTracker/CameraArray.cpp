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

	// Set up trigger and transmission mode on all cameras
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

	lock_camera_parameters();

	// Print settings for first camera
#if VERBOSE
	dc1394_featureset_t features_for_printing;
	dc1394_feature_get_all(cameras[0], &features_for_printing);
	dc1394_feature_print_all(&feature_for_printing); // second parameter for stdout?
#endif

	COUT << "(CameraArray) constructor finished" << ENDL;
}

void CameraArray::lock_camera_parameters()
{
	// Get all of the settings from first camera
	//float brightness;
	//float exposure;
	//float sharpness;
	//float balance;
	//float hue;
	//float saturation;
	//float gamma;
	//float shutter;
	//float gain;
	//float temperature;

	const int feature_count = 4;
	dc1394feature_t features[feature_count] = {
		DC1394_FEATURE_BRIGHTNESS		, 
		DC1394_FEATURE_SHUTTER			, 
		DC1394_FEATURE_GAIN				, 
		DC1394_FEATURE_TEMPERATURE		};

	float feature_absolute_values[feature_count];
	uint32_t feature_values[feature_count];

	const int features_to_disable_count = 6;
	dc1394feature_t features_to_disable[features_to_disable_count] = {
		DC1394_FEATURE_EXPOSURE			, 
		DC1394_FEATURE_SHARPNESS		, 
		DC1394_FEATURE_WHITE_BALANCE	, 
		DC1394_FEATURE_HUE				, 
		DC1394_FEATURE_SATURATION		, 
		DC1394_FEATURE_GAMMA			};
	

	// Some stuff, like exposure, should just be turned off!
	// Set everything to manual, with absolute control if possible
	for(unsigned int i=0; i<CAMERA_COUNT; i++)
	{
		for(int f=0; f<features_to_disable_count; f++)
		{
			err = dc1394_feature_set_power(cameras[i], features_to_disable[f], DC1394_OFF);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		}
	}

	// Set everything to manual, with absolute control if possible
	for(unsigned int i=0; i<CAMERA_COUNT; i++)
	{
		for(int f=0; f<feature_count; f++)
		{
			err = dc1394_feature_set_mode(cameras[i], features[f], DC1394_FEATURE_MODE_MANUAL);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			dc1394bool_t has_absolute_control;
			err =  dc1394_feature_has_absolute_control(cameras[i], features[f], &has_absolute_control);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			if(has_absolute_control)
			{
				err = dc1394_feature_set_absolute_control(cameras[i], features[f], DC1394_ON);
				if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			}
		}
	}
	// Get settings from first camera
	for(unsigned int i=0; i<1; i++)
	{
		for(int f=0; f<feature_count; f++)
		{
			dc1394bool_t has_absolute_control;
			err =  dc1394_feature_has_absolute_control(cameras[i], features[f], &has_absolute_control);
			if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			if(has_absolute_control)
			{
				err = dc1394_feature_get_absolute_value(cameras[i], features[f], &feature_absolute_values[f]);
				if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			} else {
				err = dc1394_feature_get_value(cameras[i], features[f], &feature_values[f]);
				if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			}
		}
	}
	// Write settings to the other cameras
	for(unsigned int i=1; i<camera_count; i++)
	{
		for(int f=0; f<feature_count; f++)
		{
			dc1394bool_t has_absolute_control;
			err =  dc1394_feature_has_absolute_control(cameras[i], features[f], &has_absolute_control);
			if(has_absolute_control)
			{
				err = dc1394_feature_set_absolute_value(cameras[i], features[f], feature_absolute_values[f]);
				if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			} else {
				err = dc1394_feature_set_value(cameras[i], features[f], feature_values[f]);
				if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
			}
		}
	}

   /* err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_BRIGHTNESS		, &brightness);*/
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_EXPOSURE			, &exposure);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_SHARPNESS		, &sharpness);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_WHITE_BALANCE	, &balance);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_HUE				, &hue);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_SATURATION		, &saturation);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_GAMMA			, &gamma);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_SHUTTER			, &shutter);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_GAIN				, &gain);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
	//err = dc1394_feature_get_absolute_value(cameras[0], DC1394_FEATURE_TEMPERATURE		, &temperature);
	//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;

	//// Apply settings from the first camera to the rest of the cameras
	//for(unsigned int i=1; i<CAMERA_COUNT; i++)
	//{
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_BRIGHTNESS		, brightness);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_EXPOSURE			, exposure);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_SHARPNESS		, sharpness);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_WHITE_BALANCE	, balance);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_HUE				, hue);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_SATURATION		, saturation);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_GAMMA			, gamma);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_SHUTTER			, shutter);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_GAIN				, gain);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;
		//err = dc1394_feature_set_absolute_value(cameras[i], DC1394_FEATURE_TEMPERATURE		, temperature);
		//if(err!=DC1394_SUCCESS) COUT << dc1394_error_get_string(err) << ENDL;


	//}


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
	current_frame_timestamp_tweaked = (uint64_t)(arrival_timestamp - transfer_time*1e6 - shutter/2.0f*1e6);
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
