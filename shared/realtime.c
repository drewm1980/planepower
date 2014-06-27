#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <string.h>

#include "realtime.h"

void realtime_enable() {
	system("cpufreq-set -g performance");
	int pid = (int) getpid();
	char pid_str[20];
	sprintf(pid_str, "%d", pid);
	char cmd[80];
	strcpy(cmd, "chrt -f -p 85 ");
	strcat(cmd, pid_str);
	system(cmd);
	usleep(300);
}

void realtime_disable() {
	system("cpufreq-set -g ondemand");
	int pid = getpid();
	char pid_str[20];
	sprintf(pid_str, "%d", pid);
	char cmd[80];
	strcpy(cmd, "chrt -o -p 80");
	strcat(cmd, pid_str);
	system(cmd);
	usleep(300);
}

void print_priority() {
	int which = PRIO_PROCESS;
	id_t pid;
	int ret;

	pid = getpid();
	ret = getpriority(which, 0);
	printf("Current Priority is: %i \n", ret);
}

int main (int argc, char** argv) {
	sleep(10);
	printf("Enabeling RealTime Conditions...\n");
	realtime_enable();
	print_priority();
	sleep(10);	
	printf("Disabeling RealTime Conditions...\n");
	realtime_disable();
	print_priority();
	sleep(10);
}
