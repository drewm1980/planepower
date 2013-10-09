#ifndef UDP_COMMUNCATION_H_ 
#define UDP_COMMUNCATION_H_

#include<sys/socket.h>
#include<arpa/inet.h>

struct UDP
{ 
	struct sockaddr_in si_me;
	struct sockaddr_in si_other;
	int fd;
	unsigned int fd_len;
};

int openUDPSocket(UDP *udp);

//udp client prototypes
int openUDPClientSocket(UDP *udp_client,char *server_ip,int port);
int sendUDPClientData(UDP *udp_client,void *data,size_t data_len);
int closeUDPClientSocket(UDP *udp_client);

//udp server prototypes
int openUDPServerSocket(UDP *udp_server,int port);
int receiveUDPServerData(UDP *udp_server,void *data,size_t data_len);
int closeUDPServerSocket(UDP *udp_server);

#endif /*UDP_COMMUNCATION_H__*/

