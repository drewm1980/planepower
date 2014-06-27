#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "udp_communication.h"

#ifndef DEBUG 
#define DEBUG 0
#endif


#define BUFF_SIZE 255

typedef struct{
		int port_number;
		char *server_ip;
		int file;
} Settings;



int main(int argc, char *argv[])
{
	Settings settings;
	
	//parse arguments	
	if(argc == 4){
		//first argument is always name of program or empty string
		settings.file=atoi(argv[1]);
		settings.server_ip=argv[2];
		settings.port_number=atoi(argv[3]);	
	}else{
			printf("wrong parameters: file - server ip - udp port number\n");
			exit(1);		
	}
	
    uint8_t buff[BUFF_SIZE];
    FILE *f;
     
    if(settings.file==0)
		f = fopen("data_lisa_log.txt", "r");
	else
		f = fopen("/media/sdcard/data_groundstation_log.txt", "r");
		
    if(f==NULL){
		printf("Could not open log file\n");
		exit(1);
	}
    
    int ch;
    int length;
    int i;
    static UDP udp_client;

	ch = fgetc(f);	

	openUDPClientSocket(&udp_client,settings.server_ip,settings.port_number,10000000);


while(ch!=EOF){

	if(ch!=0x99){
		printf("wrong startbyte\n");
		exit(1);
	}else{
		buff[0]=ch;
		length=fgetc(f);
		buff[1]=length;
		for(i=2;i<length;i++){
			buff[i]=fgetc(f);
		}

		#if DEBUG
		
		for(i=0;i<length;i++){
			printf("%d ",buff[i]);
		}
		printf("\n\n");
		
		#endif

		sendUDPClientData(&udp_client,&buff,length);

	}
	usleep(1000);
	ch = fgetc(f);
}
	closeUDPClientSocket(&udp_client);

    fclose(f);
    return 0;
}
