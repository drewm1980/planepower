#ifndef STEREO_PAIR_HPP
#define STEREO_PAIR_HPP

#include <dc1394/dc1394.h>

#define BROADCAST 0 // Set to true to use the camera broadcast feature.  
					// Note: Code will hang if cameras are NOT on same bus.
					// On our firewire card, plug into the first two sockets.
					// This doesn't seem to have much effect in any case.

#define TRIGGER_ACTIVE_HIGH 0
#define FRAME_BUFFER_DEPTH 4

/**
 * This class captures images from a single firewire camera,
 * and finds the location of three LED markers in the image.
 * The emphasis is on speed, rather than robustness.
 */
class StereoPair
{
	protected:
	private:
		dc1394error_t err;
		dc1394_t *dc1394_driver;
		dc1394camera_list_t *camera_list;
		dc1394camera_t *cameras[2];
		dc1394video_frame_t *current_frame[2];
		dc1394video_mode_t res;

		float f_fps; // hz
		float period; // sec

		uint64_t right_guid;
		uint64_t left_guid;
		float exposure_time; // Also called "shutter" in some interfaces.  (sec)
		float transfer_time; // Expected time to transer a set of frames over the ieee1394 bus (sec)

		uint64_t last_frame_timestamps[2]; // Timestamps of the last frame arrival (usec)
		uint64_t current_frame_timestamps[2]; // Timestamps of the frames being processed(usec)
		uint64_t current_frame_timestamp_tweaked; // Time stamp for the frame pair with compensation for hardware delays

		void configure_cameras();
		void flush_buffer(dc1394camera_t*);
		void tweak_frame_arrival_times();
		bool _useExternalTrigger;

	public:
		StereoPair(bool useExternalTrigger);
		~StereoPair();
		bool        startHook();
		void        updateHook();
		void        stopHook();
		void        cleanUpHook();
		int frame_w;
		int frame_h;
		union{
			uint8_t *current_frame_data[2]; 
			struct {
				uint8_t * right_frame_data;
				uint8_t * left_frame_data;
			};
		};
		uint64_t current_timestamp; // us
};

#endif
