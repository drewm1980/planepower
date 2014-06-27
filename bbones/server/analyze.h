#ifndef ANALYZE_H_ 
#define ANALYZE_H_

#include <sys/time.h>
#include <time.h>
#include <time_highwind.h>
#include "data_decoding.h"

typedef struct{
	double sum;
	int buffsize;
	int index;
	double avg;
	int is_first_data;
	struct timeval previous_timestamp;
	double *buffer;
}Analyze;

void init_analyze(Analyze *an,int buffsize);
int calculate_frequency(Analyze *an,struct timeval tvSent); /*calculates the freq of the package in hz*/
int calculate_latency(Analyze *an, struct timeval tvSent, struct timeval tvNow); /*calculates the latency between tvNow and tvSent*/
void dump_buffer_to_file(Analyze *an,const char *file_name);
void destroy_analyze(Analyze *an);
double get_avg(Analyze *an);
void timestamp_to_timeString(timeval tv,char time_string[]);
void timestamp_to_timeString16(Timeval16 tv,char time_string[]);

#endif /*ANALYZE_H_*/

