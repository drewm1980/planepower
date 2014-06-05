#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "time_highwind.h"

#include "udp_communication.h"
#include "uart_communication2.h" // has declaration of an error handler function pointer
#include "data_decoding.h"
#include "log.h" // also has some error handling stuff
#include "analyze.h"

#define MAX_INPUT_STREAM_SIZE 255
#define MAX_OUTPUT_STREAM_SIZE 36
#define UDP_SOCKET_TIMEOUT 1000000000


#ifndef DEBUG 
#define DEBUG 0
#endif

#ifndef ANALYZE 
#define ANALYZE 0
#endif

		
/************************************
 * PROTOTYPES
 * **********************************/

static void *server_to_planebone(void *connection);
static void print_mem(void const *vp, int n);
void write_error(char *file_name,char *message,int err_code);

 /***********************************
  * GLOBALS
  * *********************************/
  
static char FILENAME[] = "main_server.c";
 
typedef struct{
		int port_number_server_to_planebone;
		int port_number_planebone_to_server;
		char *planebone_ip;
} Connection;

//function pointer to write errors to log
void (*write_error_ptr)(char *,char *,int);

			
 /***********************************
  * MAIN
  * *********************************/
int main(int argc, char *argv[]){

	Connection connection;
	write_error_ptr = &write_error;  //initialize the function pointer to write error
	
	//parse arguments	
	if(argc == 4){
		//first argument is always name of program or empty string
		connection.planebone_ip=argv[1];
		connection.port_number_server_to_planebone=atoi(argv[2]);	
		connection.port_number_planebone_to_server=atoi(argv[3]);
	}else{
			printf("wrong parameters: planebone ip - send port number - receive port number\n");
			exit(EXIT_FAILURE);		
	}
		
	pthread_t thread_server_to_planebone;

	//create a second thread which executes server_to_planebone
	if(pthread_create(&thread_server_to_planebone, NULL, server_to_planebone,&connection)) {
		error_write(FILENAME,"error creating lisa thread");
		exit(EXIT_FAILURE);
	}	
	
	/*-------------------------START OF FIRST THREAD: PLANEBONE TO SERVER------------------------*/
	
	static UDP udp_server;
	uint8_t input_stream[MAX_INPUT_STREAM_SIZE];
	timeval tv_now;

	UDP_err_handler(openUDPServerSocket(&udp_server,connection.port_number_planebone_to_server,UDP_SOCKET_TIMEOUT),write_error_ptr);
	
	//init the data decode pointers
	init_decoding();
	
	/*
	 * WHAT WE EXPECT:
	 * IMU_ACCEL_RAW 204
	 * IMU_GYRO_RAW 203
	 * IMU_MAG_RAW 205
	 * BARO_RAW 221
	 * GPS_INT 155
	 * AIRSPEED_ETS 57
	 * SYSMON 33
	 * UART_ERROR 208
	 * ACTUATORS_received 105
	 * */
	 
	int IMU_ACCEL_RAW_received=0;
	int IMU_GYRO_RAW_received=0;	
	int IMU_MAG_RAW_received=0;
	int BARO_RAW_received=0;
	int GPS_INT_received=0;
	int AIRSPEED_received=0;
	int SVINFO_received=0;
	int SYSMON_received=0;
	int UART_ERROR_received=0;
	int ACTUATORS_received=0;
	int NMEA_IIMWV_received = 0;
	int NMEA_WIXDR_received = 0;
	int err;
	
	#if ANALYZE
		Analyze an_imu_accel_raw_freq,an_imu_accel_raw_lat;
		int an_imu_accel_freq_done=0,an_imu_accel_lat_done=0;
		init_analyze(&an_imu_accel_raw_freq,2000);
		init_analyze(&an_imu_accel_raw_lat,2000);

		Analyze an_imu_gyro_raw_freq,an_imu_gyro_raw_lat;
		int an_imu_gyro_freq_done=0,an_imu_gyro_lat_done=0;
		init_analyze(&an_imu_gyro_raw_freq,2000);
		init_analyze(&an_imu_gyro_raw_lat,2000);
		
		Analyze an_imu_mag_raw_freq,an_imu_mag_raw_lat;
		int an_imu_mag_freq_done=0,an_imu_mag_lat_done=0;
		init_analyze(&an_imu_mag_raw_freq,2000);
		init_analyze(&an_imu_mag_raw_lat,2000);
		
		Analyze an_baro_raw_freq,an_baro_raw_lat;
		int an_baro_raw_freq_done=0,an_baro_raw_lat_done=0;
		init_analyze(&an_baro_raw_freq,2000);
		init_analyze(&an_baro_raw_lat,2000);
		
		Analyze an_gps_int_freq,an_gps_int_lat;
		int an_gps_int_freq_done=0,an_gps_int_lat_done=0;
		init_analyze(&an_gps_int_freq,40);
		init_analyze(&an_gps_int_lat,40);
		
		Analyze an_airspeed_ets_freq,an_airspeed_ets_lat;
		int an_airspeed_ets_freq_done=0,an_airspeed_ets_lat_done=0;
		init_analyze(&an_airspeed_ets_freq,100);
		init_analyze(&an_airspeed_ets_lat,100);
		
		Analyze an_actuators_freq,an_actuators_lat;
		int an_actuators_freq_done=0,an_actuators_lat_done=0;
		init_analyze(&an_actuators_freq,500);
		init_analyze(&an_actuators_lat,500);
		
		Analyze an_UART_errors_freq,an_UART_errors_lat;
		int an_UART_errors_freq_done=0,an_UART_errors_lat_done=0;
		init_analyze(&an_UART_errors_freq,50);
		init_analyze(&an_UART_errors_lat,50);
		
		Analyze an_sys_mon_freq,an_sys_mon_lat;
		int an_sys_mon_freq_done=0,an_sys_mon_lat_done=0;
		init_analyze(&an_sys_mon_freq,50);
		init_analyze(&an_sys_mon_lat,50);

	#endif

	while(1){
		//1. retreive UDP data form planebone from ethernet port.
		err = receiveUDPServerData(&udp_server,(void *)&input_stream,sizeof(input_stream)); //blocking !!!
		UDP_err_handler(err,write_error_ptr); 
	
		if(err == UDP_ERR_NONE){
			gettimeofday(&tv_now,NULL); //timestamp from receiving to calculate latency
			
			#if DEBUG > 0
			
				printf("message raw: ");
				int i;
				for(i=0;i<input_stream[1];i++){
						printf("%d ",input_stream[i]);
				}
				printf("\n");
				
				printf("start hex: %x\n", input_stream[0]);
				printf("length: %d\n", input_stream[1]);
				printf("send id: %d\n", input_stream[2]);
				printf("message id: %d\n", input_stream[3]);
				printf("checksum1: %d\n", input_stream[input_stream[1]-2]);
				printf("checksum2: %d\n", input_stream[input_stream[1]-1]);
				printf("%d", input_stream[3]);
				printf("\n");
			
			#endif
			
			//2. decode data 		
			int err  = data_decode(input_stream);
			DEC_err_handler(err,write_error_ptr);
	
			if(err==DEC_ERR_NONE){ 
			
				switch_read_write(); //only switch read write if data decoding was succesfull
				Data* data = get_read_pointer();

				if(input_stream[3]==IMU_GYRO_RAW){
					IMU_GYRO_RAW_received=1;
				}else if(input_stream[3]==IMU_ACCEL_RAW){
					IMU_ACCEL_RAW_received=1;
				}else if(input_stream[3]==IMU_MAG_RAW){
					IMU_MAG_RAW_received=1;
				}else if(input_stream[3]==BARO_RAW){
					BARO_RAW_received=1;
				}else if(input_stream[3]==GPS_INT){
					GPS_INT_received=1;
				}else if(input_stream[3]==AIRSPEED_ETS){
					AIRSPEED_received=1;
				}else if(input_stream[3]==SVINFO){
					SVINFO_received=1;
				}else if(input_stream[3]==SYSMON){
					SYSMON_received=1;
				}else if(input_stream[3]==UART_ERRORS){
					UART_ERROR_received=1;
				}else if(input_stream[3]==ACTUATORS){
					ACTUATORS_received=1;
				}else if(input_stream[3]==NMEA_IIMWV_ID){
					NMEA_IIMWV_received=1;
				}else if(input_stream[3]==NMEA_WIXDR_ID){
					NMEA_WIXDR_received=1;
				}
				
				/*printf("IMU_GYRO_RAW_received %d\n",IMU_GYRO_RAW_received);
				printf("IMU_ACCEL_RAW_received %d\n",IMU_ACCEL_RAW_received);
				printf("IMU_MAG_RAW_received %d\n",IMU_MAG_RAW_received);
				printf("BARO_RAW_received %d\n",BARO_RAW_received);
				printf("GPS_INT_received %d\n",GPS_INT_received);			
				printf("AIRSPEED_received %d\n",AIRSPEED_received);			
				printf("SVINFO_received %d\n",SVINFO_received);	
				printf("SYSMON_received %d\n",SYSMON_received);	
				printf("UART_ERROR_received %d\n",UART_ERROR_received);	
				printf("ACTUATORS_received %d\n",ACTUATORS_received);
				printf("NMEA_IIMWV_received %d\n",NMEA_IIMWV_received);	
				printf("NMEA_WIXDR_received %d\n",NMEA_WIXDR_received);	
				printf("\n");*/

				if(input_stream[3]==BARO_RAW){
					#if ANALYZE
						if(calculate_frequency(&an_baro_raw_freq,data->lisa_plane.baro_raw.tv)==1){
							an_baro_raw_freq_done=1;
						}

						if(calculate_latency(&an_baro_raw_lat,data->lisa_plane.baro_raw.tv,tv_now)==1){
							an_baro_raw_lat_done=1;
						}
					#endif
					/*int i;
					printf("Baro_raw content:");	
					print_mem((void *)&data->lisa_plane.baro_raw,sizeof(Baro_raw));
	
					printf("abs %d\n",data->lisa_plane.baro_raw.abs);
					printf("diff %d\n",data->lisa_plane.baro_raw.diff);	
					
									 
					printf("\n\n\n");*/
				}
				
				if(input_stream[3]==IMU_GYRO_RAW){
					
					#if ANALYZE
						if(calculate_frequency(&an_imu_gyro_raw_freq,data->lisa_plane.imu_gyro_raw.tv)==1){
							an_imu_gyro_freq_done=1;
						}
						

						if(calculate_latency(&an_imu_gyro_raw_lat,data->lisa_plane.imu_gyro_raw.tv,tv_now)==1){
							an_imu_gyro_lat_done=1;
						}
					#endif
					
				/*	int i;
					printf("Imu_gyro_raw content:");
					print_mem((void *)&data->lisa_plane.imu_gyro_raw,sizeof(Imu_gyro_raw));
				
					printf("\n");
					printf("gp %d\n",data->lisa_plane.imu_gyro_raw.gp);
					printf("gq %d\n",data->lisa_plane.imu_gyro_raw.gq);
					printf("gr %d\n",data->lisa_plane.imu_gyro_raw.gr);
					
					print_latency(data->lisa_plane.imu_gyro_raw.tv);
				 
					printf("\n\n\n");*/

				}
				
				if(input_stream[3]==IMU_ACCEL_RAW){
					
					#if ANALYZE
						if(calculate_frequency(&an_imu_accel_raw_freq,data->lisa_plane.imu_accel_raw.tv)==1){
							an_imu_accel_freq_done=1;
						}
						

						if(calculate_latency(&an_imu_accel_raw_lat,data->lisa_plane.imu_accel_raw.tv,tv_now)==1){
							an_imu_accel_lat_done=1;
						}
					#endif
					
					
					/*int i;
					printf("Imu_accel_raw content:");
					print_mem((void *)&data->lisa_plane.imu_accel_raw,sizeof(Imu_accel_raw));


					printf("\n");
					printf("ax %d\n",data->lisa_plane.imu_accel_raw.ax);
					printf("ay %d\n",data->lisa_plane.imu_accel_raw.ay);
					printf("az %d\n",data->lisa_plane.imu_accel_raw.az);
					printf("\n");
					
			 
					printf("\n\n\n");*/

				}
				
				if(input_stream[3]==IMU_MAG_RAW){
					
					#if ANALYZE
						if(calculate_frequency(&an_imu_mag_raw_freq,data->lisa_plane.imu_mag_raw.tv)==1){
							an_imu_mag_freq_done=1;
						}

						if(calculate_latency(&an_imu_mag_raw_lat,data->lisa_plane.imu_mag_raw.tv,tv_now)==1){
							an_imu_mag_lat_done=1;
						}
					#endif
						

				}
				
				if(input_stream[3]==AIRSPEED_ETS){
					
					#if ANALYZE
						if(calculate_frequency(&an_airspeed_ets_freq,data->lisa_plane.airspeed_ets.tv)==1){
							an_airspeed_ets_freq_done=1;
						}

						if(calculate_latency(&an_airspeed_ets_lat,data->lisa_plane.airspeed_ets.tv,tv_now)==1){
							an_airspeed_ets_lat_done=1;
						}
					#endif
					
					/*int i;
					printf("airspeed content:");
					print_mem((void *)&data->lisa_plane.airspeed_ets,sizeof(Airspeed_ets));
					
					char temp[64];
					timestamp_to_timeString(data->lisa_plane.airspeed_ets.tv,temp);
					printf("send time %s\n",temp);

					printf("adc %d\n",data->lisa_plane.airspeed_ets.adc);
					printf("offset %d\n",data->lisa_plane.airspeed_ets.offset);
					printf("scaled %f\n",data->lisa_plane.airspeed_ets.scaled);*/
				}
				
				
				if(input_stream[3]==GPS_INT){
					#if ANALYZE
						if(calculate_frequency(&an_gps_int_freq,data->lisa_plane.gps_int.tv)==1){
							an_gps_int_freq_done=1;
						}

						if(calculate_latency(&an_gps_int_lat,data->lisa_plane.gps_int.tv,tv_now)==1){
							an_gps_int_lat_done=1;
						}
					#endif
				
					/*int i;
					printf("Gps_int_message content:");
					print_mem((void *)&data->lisa_plane.gps_int,sizeof(Gps_int));

					printf("\n");
					printf("ecef_x %d\n",data->lisa_plane.gps_int.ecef_x);
					printf("ecef_y %d\n",data->lisa_plane.gps_int.ecef_y);
					printf("ecef_z %d\n",data->lisa_plane.gps_int.ecef_z);
					printf("lat %d\n",data->lisa_plane.gps_int.lat);
					printf("lon %d\n",data->lisa_plane.gps_int.lon);
					printf("alt %d\n",data->lisa_plane.gps_int.alt);
					printf("hmsl %d\n",data->lisa_plane.gps_int.hmsl);
					printf("ecef_xd %d\n",data->lisa_plane.gps_int.ecef_xd);
					printf("ecef_yd %d\n",data->lisa_plane.gps_int.ecef_yd);
					printf("ecef_zd %d\n",data->lisa_plane.gps_int.ecef_zd);
					printf("pacc %d\n",data->lisa_plane.gps_int.pacc);
					printf("sacc %d\n",data->lisa_plane.gps_int.sacc);
					printf("tow %d\n",data->lisa_plane.gps_int.tow);
					printf("pdop %d\n",data->lisa_plane.gps_int.pdop);
					printf("numsv %d\n",data->lisa_plane.gps_int.numsv);
					printf("fix %d\n",data->lisa_plane.gps_int.fix);
					print_latency(data->lisa_plane.gps_int.tv);*/

					
				}
				
		
				if(input_stream[3]==SYSMON){
					
					#if ANALYZE
						if(calculate_frequency(&an_sys_mon_freq,data->lisa_plane.sys_mon.tv)==1){
							an_sys_mon_freq_done=1;
						}

						if(calculate_latency(&an_sys_mon_lat,data->lisa_plane.sys_mon.tv,tv_now)==1){
							an_sys_mon_lat_done=1;
						}
					#endif	
					
				/*	int i;
					printf("sysmon content:");
					print_mem((void *)&data->lisa_plane.sys_mon,sizeof(Sys_mon));

					printf("\n");
					printf("periodic_time %d\n",data->lisa_plane.sys_mon.periodic_time);
					printf("periodic_cycle %d\n",data->lisa_plane.sys_mon.periodic_cycle);
					printf("periodic_cycle_min %d\n",data->lisa_plane.sys_mon.periodic_cycle_min);
					printf("periodic_cycle_max %d\n",data->lisa_plane.sys_mon.periodic_cycle_max);
					printf("event_number %d\n",data->lisa_plane.sys_mon.event_number);
					printf("cpu_load %d\n",data->lisa_plane.sys_mon.cpu_load);
					print_latency(data->lisa_plane.sys_mon.tv);*/

				}
			if(input_stream[3]==UART_ERRORS){
				
				#if ANALYZE
					if(calculate_frequency(&an_UART_errors_freq,data->lisa_plane.uart_errors.tv)==1){
						an_UART_errors_freq_done=1;
					}

					if(calculate_latency(&an_UART_errors_lat,data->lisa_plane.uart_errors.tv,tv_now)==1){
						an_UART_errors_lat_done=1;
					}
				#endif
				/*	int i;
					printf("uart error content:");
					print_mem((void *)&data->lisa_plane.uart_errors,sizeof(UART_errors));

					printf("overrun_cnt %d\n",data->lisa_plane.uart_errors.overrun_cnt);
					printf("noise_err_cnt %d\n",data->lisa_plane.uart_errors.noise_err_cnt);
					printf("framing_err_cnt %d\n",data->lisa_plane.uart_errors.framing_err_cnt);
					printf("bus_number %d\n",data->lisa_plane.uart_errors.bus_number);
					print_latency(data->lisa_plane.uart_errors.tv);*/

				}
				if(input_stream[3]==ACTUATORS){
					#if ANALYZE
						if(calculate_frequency(&an_actuators_freq,data->lisa_plane.actuators.tv)==1){
							an_actuators_freq_done=1;
						}

						if(calculate_latency(&an_actuators_lat,data->lisa_plane.actuators.tv,tv_now)==1){
							an_actuators_lat_done=1;
						}
					#endif
					
					/*int i;
					printf("actuators content:");
					print_mem((void *)&data->lisa_plane.actuators,sizeof(Actuators));

					printf("arr_length %d\n",data->lisa_plane.actuators.arr_length);
					for(i=0;i<data->lisa_plane.actuators.arr_length;i++){
						printf("servo_%d %d\n",i,data->lisa_plane.actuators.values[i]);
					}
					print_latency(data->lisa_plane.actuators.tv);*/

				}
				
				if(input_stream[3]==BEAGLE_ERROR){
					//printf("beagle bone error content:");
					//print_mem((void *)&data->bone_plane.error,sizeof(Beagle_error));
										
					switch(data->bone_plane.error.library){
						case UDP_L:
							UDP_err_handler(data->bone_plane.error.error_code,write_error_ptr);
						break;
						case UART_L:
							UART_err_handler(data->bone_plane.error.error_code,write_error_ptr);
						break;
						case DECODE_L:
							DEC_err_handler(data->bone_plane.error.error_code,write_error_ptr);
						break;
						case LOG_L:
							LOG_err_handler(data->bone_plane.error.error_code,write_error_ptr);
						break;
					}
				}
				
				if(input_stream[3]==NMEA_IIMWV_ID){
					
					printf("NMEA_IIMWV_ID content:");
					print_mem((void *)&data->bone_wind.nmea_iimmwv,sizeof(NMEA_IIMWV));

					printf("wind angle %lf\n",data->bone_wind.nmea_iimmwv.wind_angle);
					printf("relative %c\n",data->bone_wind.nmea_iimmwv.relative);
					printf("wind speed %lf\n",data->bone_wind.nmea_iimmwv.wind_speed);
					printf("wind speed unit %c\n",data->bone_wind.nmea_iimmwv.wind_speed_unit);
					printf("status %c\n",data->bone_wind.nmea_iimmwv.status);
					char temp[64];
					timestamp_to_timeString16(data->bone_wind.nmea_iimmwv.tv,temp);
					printf("send time: %s\n",temp);
					printf("\n");

				}
				
				if(input_stream[3]==NMEA_WIXDR_ID){
					
					printf("NMEA_WIXDR_ID content:");
					print_mem((void *)&data->bone_wind.nmea_wixdr,sizeof(NMEA_WIXDR));
					
					printf("Temperature %lf\n",data->bone_wind.nmea_wixdr.temperature);
					printf("unit %c\n",data->bone_wind.nmea_wixdr.unit);
					char temp[64];
					timestamp_to_timeString16(data->bone_wind.nmea_wixdr.tv,temp);
					printf("send time: %s\n",temp);
					printf("\n");
				}
				
				if(input_stream[3]==LINE_ANGLE_ID){
					
					printf("LINE_ANGLE_ID content:");
					print_mem((void *)&data->bone_arm.line_angle,sizeof(LINE_ANGLE));
					
					printf("Azimuth %lf\n",data->bone_arm.line_angle.temperature);
					printf("unit %c\n",data->bone_wind.nmea_wixdr.unit);
					char temp[64];
					timestamp_to_timeString16(data->bone_wind.nmea_wixdr.tv,temp);
					printf("send time: %s\n",temp);
					printf("\n");
				}
									
			}else{
					printf("UNKNOW PACKAGE with id %d\n",input_stream[3]);
					exit(1);
			}
		}
		#if ANALYZE
			if(an_imu_accel_freq_done==1 && 
					an_imu_accel_lat_done==1 && 
					an_imu_gyro_freq_done==1 && 
					an_imu_gyro_lat_done==1 && 
					an_imu_mag_freq_done==1 && 
					an_imu_mag_lat_done==1 && 
					an_baro_raw_lat_done==1 && 
					an_baro_raw_freq_done==1 && 
					an_airspeed_ets_lat_done==1 && 
					an_airspeed_ets_freq_done==1 && 
					an_actuators_lat_done==1 && 
					an_actuators_freq_done==1 && 
					an_UART_errors_lat_done==1 && 
					an_UART_errors_freq_done==1 && 
					an_sys_mon_lat_done==1 && 
					an_sys_mon_freq_done==1)
			{
				printf("ANALYZE RESULTS - IMU_ACCEL_RAW:\n");
				printf("avg period:\t %0.4f ms\n",(get_avg(&an_imu_accel_raw_freq)));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_imu_accel_raw_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_imu_accel_raw_lat));
				printf("\n");
				dump_buffer_to_file(&an_imu_accel_raw_freq,"analyze/imu_accel_raw_per.csv");
				dump_buffer_to_file(&an_imu_accel_raw_lat,"analyze/imu_accel_raw_lat.csv");
				destroy_analyze(&an_imu_accel_raw_freq);
				destroy_analyze(&an_imu_accel_raw_lat);
				
				printf("ANALYZE RESULTS - IMU_GYRO_RAW:\n");
				printf("avg period:\t %0.4f ms\n",(get_avg(&an_imu_gyro_raw_freq)));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_imu_gyro_raw_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_imu_gyro_raw_lat));
				printf("\n");
				dump_buffer_to_file(&an_imu_gyro_raw_freq,"analyze/imu_gyro_raw_per.csv");
				dump_buffer_to_file(&an_imu_gyro_raw_lat,"analyze/imu_gyro_raw_lat.csv");
				destroy_analyze(&an_imu_gyro_raw_freq);
				destroy_analyze(&an_imu_gyro_raw_lat);
				
				printf("ANALYZE RESULTS - IMU_MAG_RAW:\n");
				printf("avg period:\t %0.4f ms\n",(get_avg(&an_imu_mag_raw_freq)));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_imu_mag_raw_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_imu_mag_raw_lat));
				printf("\n");
				dump_buffer_to_file(&an_imu_mag_raw_freq,"analyze/imu_mag_raw_per.csv");
				dump_buffer_to_file(&an_imu_mag_raw_lat,"analyze/imu_mag_raw_lat.csv");
				destroy_analyze(&an_imu_mag_raw_freq);
				destroy_analyze(&an_imu_mag_raw_lat);
				
				printf("ANALYZE RESULTS - BARO_RAW:\n");
				printf("avg period:\t %0.4f ms\n",(get_avg(&an_baro_raw_freq)));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_baro_raw_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_baro_raw_lat));
				printf("\n");
				dump_buffer_to_file(&an_baro_raw_freq,"analyze/baro_raw_per.csv");
				dump_buffer_to_file(&an_baro_raw_lat,"analyze/baro_raw_lat.csv");
				destroy_analyze(&an_baro_raw_freq);
				destroy_analyze(&an_baro_raw_lat);
				
				printf("ANALYZE RESULTS - GPS_INT:\n");
				printf("avg period:\t %0.4f ms\n",get_avg(&an_gps_int_freq));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_gps_int_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_gps_int_lat));
				printf("\n");
				dump_buffer_to_file(&an_gps_int_freq,"analyze/gps_int_per.csv");
				dump_buffer_to_file(&an_gps_int_lat,"analyze/gps_int_lat.csv");
				destroy_analyze(&an_gps_int_freq);
				destroy_analyze(&an_gps_int_lat);
				
				printf("ANALYZE RESULTS - AIRSPEED_ETS:\n");
				printf("avg period:\t %0.4f ms\n",get_avg(&an_airspeed_ets_freq));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_airspeed_ets_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_airspeed_ets_lat));
				printf("\n");
				dump_buffer_to_file(&an_airspeed_ets_freq,"analyze/airspeed_ets_per.csv");
				dump_buffer_to_file(&an_airspeed_ets_lat,"analyze/airspeed_ets_lat.csv");
				destroy_analyze(&an_airspeed_ets_freq);
				destroy_analyze(&an_airspeed_ets_lat);
				
				printf("ANALYZE RESULTS - ACTUATORS:\n");
				printf("avg period:\t %0.4f ms\n",get_avg(&an_actuators_freq));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_actuators_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_actuators_lat));
				printf("\n");
				dump_buffer_to_file(&an_actuators_freq,"analyze/actuators_per.csv");
				dump_buffer_to_file(&an_actuators_lat,"analyze/actuators_lat.csv");
				destroy_analyze(&an_actuators_freq);
				destroy_analyze(&an_actuators_lat);
				
				printf("ANALYZE RESULTS - UART_ERRORS:\n");
				printf("avg period:\t %0.4f ms\n",get_avg(&an_UART_errors_freq));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_UART_errors_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_UART_errors_lat));
				printf("\n");
				dump_buffer_to_file(&an_UART_errors_freq,"analyze/UART_errors_per.csv");
				dump_buffer_to_file(&an_UART_errors_lat,"analyze/UART_errors_lat.csv");
				destroy_analyze(&an_UART_errors_freq);
				destroy_analyze(&an_UART_errors_lat);
			
				printf("ANALYZE RESULTS - SYS_MON:\n");
				printf("avg period:\t %0.4f ms\n",get_avg(&an_sys_mon_freq));
				printf("avg freq:\t %0.4f hz\n",1/(get_avg(&an_sys_mon_freq))*1e3);
				printf("avg latency\t %0.4f ms\n",get_avg(&an_sys_mon_lat));
				printf("\n");
				dump_buffer_to_file(&an_sys_mon_freq,"analyze/sys_mon_per.csv");
				dump_buffer_to_file(&an_sys_mon_lat,"analyze/sys_mon_lat.csv");
				destroy_analyze(&an_sys_mon_freq);
				destroy_analyze(&an_sys_mon_lat);

				exit(1);
			}
		#endif
		
	}
	
	UDP_err_handler(closeUDPServerSocket(&udp_server),write_error_ptr);

	/*------------------------END OF FIRST THREAD------------------------*/

	//wait for the second thread to finish
	if(pthread_join(thread_server_to_planebone, NULL)) {
		error_write(FILENAME,"error joining thread_lisa_to_pc");
		exit(EXIT_FAILURE);
	}

	return 0;
}

static void *server_to_planebone(void *connection){
/*-------------------------START OF SECOND THREAD: SERVER TO PLANEBONE------------------------*/		
	
	Connection *conn=(Connection *)connection;
	static UDP udp_client;
	//1. read data from i don't now where
	uint8_t encoded_data[MAX_OUTPUT_STREAM_SIZE];
	
	UDP_err_handler(openUDPClientSocket(&udp_client,conn->planebone_ip,conn->port_number_server_to_planebone,UDP_SOCKET_TIMEOUT),write_error_ptr);
	int i=0;
	
	while(1)
	{
		
		Output output;

		//create test data
		output.message.servo_1=-i;
		output.message.servo_2=i;
		output.message.servo_3=i;
		output.message.servo_4=i;
		output.message.servo_5=i;
		output.message.servo_6=i;
		output.message.servo_7=0;
		i=i+900;
		if(i>12800){
			i=0;	
		}
	
		//2. encode the data	
		DEC_err_handler(data_encode(output.raw,sizeof(output.raw),encoded_data,SERVER,SERVO_COMMANDS),write_error_ptr);
	
		/*printf("OUTPUT RAW:");
		int j;
			for(j=0;j<encoded_data[1];j++){
				printf("%d ",encoded_data[j]);
			}
		printf("\n");*/

		//3. send data to eth port using UDP
		UDP_err_handler(sendUDPClientData(&udp_client,&encoded_data,sizeof(encoded_data)),write_error_ptr);	
		
		//usleep(20000); //60 hz
		
		sleep(1);
	}
	UDP_err_handler(closeUDPClientSocket(&udp_client),write_error_ptr);



	return NULL;
/*------------------------END OF SECOND THREAD------------------------*/
}	

void write_error(char *file_name,char *message,int err_code)
{
	//TODO: make it thread safe!!
    error_write(file_name,message);
}

static void print_mem(void const *vp, int n)
{
    unsigned char const *p = vp;
	int i;
	
    for (i=0; i<n; i++)
        printf("%d ", p[i]);
    printf("\n");
};




