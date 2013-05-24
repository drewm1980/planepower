#ifndef CAMERA_ARRAY_HPP
#define CAMERA_ARRAY_HPP

#include <dc1394/dc1394.h>

#define BROADCAST 0 // Set to true to use the camera broadcast feature.  
					// Note: Code will hang if cameras are NOT on same bus.
					// On our firewire card, plug into the first two sockets.
					// This doesn't seem to have much effect in any case.

#define TRIGGER_ACTIVE_HIGH 0
#define FRAME_BUFFER_DEPTH 4

#define CAMERA_COUNT 3 // It is a hard error to NOT have this many cameras.

#if 0
#define VIDEO_MODE DC1394_VIDEO_MODE_1600x1200_MONO8
#define FRAME_W 1600
#define FRAME_H 1200
#else
#define VIDEO_MODE DC1394_VIDEO_MODE_800x600_RGB8
#define FRAME_W 800
#define FRAME_H 600
#endif

#define RIGHT_GUID 0xb09d0100a98977
#define LEFT_GUID  0xb09d0100a98974
#define CENTER_GUID 0xb09d0100a9897b

#if CAMERA_COUNT==2
#define GUIDS {RIGHT_GUID,LEFT_GUID}
#else
#define GUIDS {LEFT_GUID,CENTER_GUID,RIGHT_GUID}
#endif

#define EXPOSURE_TIME_AT_STARTUP .003f

/**
 * This class configures and captures images from multiple firewire cameras.
 */
class CameraArray
{
	protected:
	private:
		dc1394error_t err;
		dc1394_t *dc1394_driver;
		dc1394camera_list_t *camera_list;
		dc1394camera_t *cameras[CAMERA_COUNT];
		dc1394video_frame_t *current_frame[CAMERA_COUNT];
		dc1394video_mode_t res;

		float f_fps; // hz
		float period; // sec

		uint64_t guids[CAMERA_COUNT];
		float exposure_time; // Also called "shutter" in some interfaces.  (sec)
		float transfer_time; // Expected time to transer a complete set of frames over the ieee1394 bus (sec)

		uint64_t last_frame_timestamps[CAMERA_COUNT]; // Timestamps of the last frame arrival (usec)
		uint64_t current_frame_timestamps[CAMERA_COUNT]; // Timestamps of the frames being processed(usec)
		uint64_t current_frame_timestamp_tweaked; // Time stamp for the frameset with compensation for hardware delays

		void configure_cameras();
		void flush_buffer(dc1394camera_t*);
		void tweak_frame_arrival_times();
		bool _useExternalTrigger;

	public:
		CameraArray(bool useExternalTrigger);
		~CameraArray();
		bool        startHook();
		void        updateHook();
		void        stopHook();
		void        cleanUpHook();
		int frame_w;
		int frame_h;
		int camera_count;
		uint8_t *current_frame_data[CAMERA_COUNT]; 
		uint64_t current_timestamp; // us
};

#endif
