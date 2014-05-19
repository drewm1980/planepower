// This header enables RealTime conditions

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

void enableRT() {
	system("cpufreq-set -g performance");
	int pid = (int) getpid();
	char pid_str[20];
	sprintf(pid_str, "%d", pid);
	char cmd[80];
	strcpy(cmd, "chrt -f -p 99 ");
	strcat(cmd, pid_str);
	system(cmd);
	usleep(300);
}

void disableRT() {
	system("cpufreq-set -g ondemand");
	int pid = getpid();
	char pid_str[20];
	sprintf(pid_str, "%d", pid);
	char cmd[80];
	strcpy(cmd, "chrt -o -p ");
	strcat(cmd, pid_str);
	system(cmd);
	usleep(300);
}

void printPrio() {
	int which = PRIO_PROCESS;
	id_t pid;
	int ret;

	pid = getpid();
	ret = getpriority(which, 0);
	printf("Current Priority is: %i \n", ret);
}
