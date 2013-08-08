#ifndef DATA_DECODING_H_ 
#define DATA_DECODING_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

/********************************
 * GLOBALS
 * ******************************/
 
// Decoding error codes 
enum dec_errCode { DEC_ERR_NONE = 0,DEC_ERR_START_BYTE,DEC_ERR_CHECKSUM,DEC_ERR_UNKNOWN_BONE_PACKAGE,DEC_ERR_UNKNOWN_LISA_PACKAGE,DEC_ERR_UNKNOWN_SENDER,DEC_ERR_UNDEFINED};
typedef enum dec_errCode DEC_errCode;

 
enum Library {UDP_L,UART_L,DECODE_L,LOG_L,CIRCULAR_BUFFER_L};
typedef enum Library library; 

//OUTPUT 
typedef union{ //message id 72
	uint8_t raw[14]; 
	struct Output_message {
			int16_t servo_1;
			int16_t servo_2;
			int16_t servo_3;
			int16_t servo_4;
			int16_t servo_5;
			int16_t servo_6;
			int16_t servo_7;
		} message;
} Output;

	
typedef union { // id = 1
		uint8_t raw[155];
		struct Status_message {
			uint8_t error;
			time_t time;
			struct timeval tv;
			int8_t new_data;
		} message;
	} Status;
	
typedef union { // id = 2
		uint8_t raw[27];
		struct Error_message {
			uint8_t library;
			uint8_t error;
			time_t time; //8
			struct timeval tv; //16
			int8_t new_data;
		} message;
	} LisaError;
	
typedef struct { // sender id = 2
		Status status;
		LisaError error;
	} Bone_plane;	
	
typedef union { // id = 54
		uint8_t raw[9];
		struct Airspeed_message {
			float airspeed;
			float airspeed_sp;
			float airspeed_cnt;
			float groundspeed_sp;
			time_t time; 
			struct timeval tv; 
			int8_t new_data;
		} message;
	} Airspeed;
	
typedef union { // id = 25
		uint8_t raw[33];
		struct Svinfo_message {
			uint8_t chn;
			uint8_t svid;
			uint8_t flags;
			uint8_t qi;
			uint8_t cno;
			int8_t elev;
			uint16_t azim;
			time_t time; 
			struct timeval tv; 
			int8_t new_data;
		} message;
	} Svinfo;
	
typedef union Airspeed_ets { // id = 57
		uint8_t raw[9];
		struct Airspeed_ets_message {
			uint16_t adc;
			uint16_t offset;
			float scaled;
			time_t time;
			struct timeval tv;
			int8_t new_data;
		} message;
	} Airspeed_ets;
	
typedef union { // id = 155
		uint8_t raw[9];
		struct Gps_int_message {
			int32_t ecef_x;
			int32_t ecef_y;
			int32_t ecef_z;
			int32_t lat;
			int32_t lon;
			int32_t alt;
			int32_t hmsl;
			int32_t ecef_xd;
			int32_t ecef_yd;
			int32_t ecef_zd;
			int32_t pacc;
			int32_t sacc;
			uint32_t tow;
			uint16_t pdop;
			uint8_t numsv;
			uint8_t fix;
			time_t time;
			struct timeval tv;
			int8_t new_data;
		} message;
	} Gps_int;
	
typedef union { // id = 221
		uint8_t raw[9];
		struct Baro_raw_message {
			int32_t abs;
			int32_t diff;
			time_t time;
			struct timeval tv;
			int8_t new_data;
		} message;
	} Baro_raw;
	
typedef union { // id = 203
		uint8_t raw[9];
		struct Imu_gyro_message {
			int32_t gp;
			int32_t gq;
			int32_t gr;
			time_t time;
			struct timeval tv;
			int8_t new_data;
		} message;
	} Imu_gyro_raw;
	
typedef union { // id = 204
		uint8_t raw[9];
		struct Imu_accel_message {
			int32_t ax;
			int32_t ay;
			int32_t az;
			time_t time;
			struct timeval tv;
			int8_t new_data;
		} message;
	} Imu_accel_raw;
	
typedef union { // id = 205
		uint8_t raw[9];
		struct Imu_mag_message {
			int32_t mx;
			int32_t my;
			int32_t mz;
			time_t time;
			struct timeval tv;
			int8_t new_data;
		} message;
	} Imu_mag_raw;
	
	typedef struct { // sender id = 165
		Airspeed airspeed;
		Svinfo svinfo;
		Airspeed_ets airspeed_ets;
		Gps_int gps_int;
		Baro_raw baro_raw;
		Imu_gyro_raw imu_gyro_raw;
		Imu_accel_raw imu_accel_raw;
		Imu_mag_raw imu_mag_raw;
	} Lisa_plane;	
	
//INPUT
typedef struct
{
	Bone_plane bone_plane;
	Lisa_plane lisa_plane;
} Data;

/********************************
 * PROTOTYPES PUBLIC
 * ******************************/
void init_decoding(void);
DEC_errCode data_update(uint8_t stream[]);
int32_t data_write(uint8_t stream[], uint8_t data[], int length, int pos);
void switch_read_write(void);
DEC_errCode data_encode(uint8_t message[],long unsigned int message_length,uint8_t encoded_data[],int sender_id,int message_id);
Data* get_read_pointer();

#ifdef __cplusplus
}
#endif

#endif /*DATA_DECODING_H_*/
