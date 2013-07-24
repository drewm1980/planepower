#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

pid_t gettid(void)
{ 
	return(syscall(186)); 
}

void gdb_me(void)			                                                                            
{
	printf("Entered gdb_me script.\n");

	int pid = getpid();
	int ppid = getppid();
	int tid = gettid();
	printf("pid: %i\n", pid);
	printf("ppid: %i\n", ppid);
	printf("tid: %i\n", tid);

    //typedef struct {
               //const char *dli_fname;  [> Pathname of shared object that
                                          //contains address */
               //void       *dli_fbase;  [> Address at which shared object
                                          //is loaded */
               //const char *dli_sname;  [> Name of nearest symbol with address
                                          //lower than addr */
               //void       *dli_saddr;  [> Exact address of symbol named
                                          //in dli_sname */
           //} Dl_info;


	//DL_info info;
	//dladdr( &gdb_me, &info );
	//printf("Program name might be: %s\n", info.dli_fname);
	
	char exename[300];	 // i.e. /2936wljterhntn/7po9w6/r8o/8w6/gnulinux-deployer
	int nbytes = readlink("/proc/self/exe", exename, 300);
	exename[nbytes] = '\0'; // readlink doesn't append a null charachter.
	//printf("Program name might be: %s\n", exename);

	if(!fork())
	{
		char    cmd[100];
		//sprintf(cmd, "xterm -e bash -c \"gdb -nx -ex 'attach %d' -ex continue ; bash\"&", pid);
		//sprintf(cmd, "xterm -e bash -c \"gdb -nx -ex 'attach %d' ; bash\"&", tid);
		sprintf(cmd, "xterm -e bash -c \"gdb -nx -ex 'attach %d' -ex 'thread %d'; bash\"&", pid, tid);
		//sprintf(cmd, "xterm -e bash -c \"gdb -nx -e 'attach %d' ; bash\"&", tid);
		//sprintf(cmd, "xterm -e bash -c \"gdb -nx %s %d;\" ", exename, tid);

		printf(" About to execute %s\n", cmd);

		int err = system(cmd); err=0;
		exit(0);
	}
	else
	{
		printf("[%d] waiting for attach from gdb...\n", gettid());
		sleep(1000);
		printf("continuing now\n");
	}
}

