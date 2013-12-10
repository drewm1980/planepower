/*
 * AUTHOR: Jonas Van Pelt
 */
 
#include <stdlib.h>
#include <stdio.h>
#include "analyze.h"

#ifndef DEBUG 
#define DEBUG 0
#endif

/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
static int timeval_subtract(timeval *result,timeval *t2,timeval *t1);
 
/********************************
 * FUNCTIONS
 * ******************************/
 void init_analyze(Analyze *an,int buffsize){	 
	#if DEBUG  > 1
		printf("Entering init_analyze\n");
	#endif
	 
	an->buffsize=buffsize;
	an->sum=0;
	an->avg=0;
	an->index=0;

	an->is_first_data=1;
	an->buffer=(double *)malloc(sizeof(double)*buffsize);
	if(an->buffer == NULL){
			printf("could not allocate memory for buffer\n");
	}
}
 
int calculate_frequency(Analyze *an,timeval tvSent){
	#if DEBUG  > 1
		printf("Entering calculate_frequency\n");
	#endif
	
	if(an->index < an->buffsize){
		
		if(an->is_first_data){
			an->is_first_data=0;
		}else{
			timeval tvResult;
			double diff;
			double freq;
			timeval_subtract(&tvResult,&tvSent,&an->previous_timestamp);
			diff = ((double)tvResult.tv_sec * 1e6 + tvResult.tv_usec) * 1e-3;
			//freq=1/diff*1e3;
			an->buffer[an->index]=diff;
			an->sum+=diff;
			an->index++;
		}
		an->previous_timestamp=tvSent;
		return 0;
	}else{
		//calculate avg when buffer is full
		an->avg=an->sum/(an->buffsize);
		return 1;
	}
	
}

int calculate_latency(Analyze *an,timeval tvSent,timeval tvNow){
	#if DEBUG  > 1
		printf("Entering calculate_latency\n");
	#endif
	
	if(an->index < an->buffsize){
		timeval tvResult;
		double diff; 
		/*char tmp[60];
		timestamp_to_timeString(tvSent,tmp);
		printf("send time stamp %s\n",tmp);
		timestamp_to_timeString(tvNow,tmp);
		printf("receive time stamp %s\n\n",tmp);*/
		timeval_subtract(&tvResult,&tvNow,&tvSent);
		diff = ((double)tvResult.tv_sec * 1e6 + tvResult.tv_usec) * 1e-3;
		an->buffer[an->index]=diff;
		an->sum+=diff;
		an->index++;
		return 0;
	}else{
		//calculate avg when buffer is full
		an->avg=an->sum/(an->buffsize);
		return 1;
	}
	
}

double get_avg(Analyze *an){
	#if DEBUG  > 1
		printf("Entering get_avg\n");
	#endif	
	return an->avg;
}

void timestamp_to_timeString(timeval tv,char time_string[]){
	#if DEBUG  > 1
		printf("Entering timestamp_to_timeString\n");
	#endif
		
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64];
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(time_string, 64, "%s.%06d", tmbuf, (int)tv.tv_usec);
}


void dump_buffer_to_file(Analyze *an,const char *file_name){
	#if DEBUG  > 1
		printf("Entering dump_buffer_to_file\n");
	#endif
	
	FILE *file; 
	int i;
	file = fopen(file_name,"w"); 
	for(i=0;i<an->buffsize;i++){
		fprintf(file,"%f\n", an->buffer[i]);
	}
	fclose(file);
}

void destroy_analyze(Analyze *an){
	#if DEBUG  > 1
		printf("Entering destroy_analyze\n");
	#endif	
	an->buffsize=0;
	an->sum=0;
	an->avg=0;
	free(an->buffer);
}

static int timeval_subtract(timeval *result,timeval *t2,timeval *t1){
	#if DEBUG  > 1
		printf("Entering timeval_subtract\n");
	#endif
		
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
    return (diff<0);	//return 1 if the difference is negative, otherwise 0. 
}
