/*
 * AUTHOR: Jonas Van Pelt
 */
#ifndef ANALYZE_H_ 
#define ANALYZE_H_

#include <sys/time.h>
#include <time.h>

/********************************
 * GLOBALS
 * ******************************/
typedef struct timeval timeval;

typedef struct{
	double sum;
	int buffsize;
	int index;
	double avg;
	int is_first_data;
	timeval previous_timestamp;
	double *buffer;
}Analyze;

/********************************
 * PROTOTYPES PUBLIC
 * ******************************/
 
extern void init_analyze(Analyze *an,int buffsize);
extern int calculate_frequency(Analyze *an,timeval tvSent); /*calculates the freq of the package in hz*/
extern int calculate_latency(Analyze *an,timeval tvSent,timeval tvNow); /*calculates the latency between tvNow and tvSent*/
extern void dump_buffer_to_file(Analyze *an,const char *file_name);
extern void destroy_analyze(Analyze *an);
extern double get_avg(Analyze *an);
#endif /*ANALYZE_H_*/

