/*
 * AUTHOR: Jonas Van Pelt
 */

#ifndef UDP_COMMUNCATION_H_ 
#define UDP_COMMUNCATION_H_

#include<sys/socket.h>
#include<arpa/inet.h>

typedef struct 
{ 
	struct sockaddr_in si_me;
	struct sockaddr_in si_other;
	int fd;
	int fd_len;
} UDP;

extern int openUDPSocket(UDP *udp);

//udp client prototypes
extern int openUDPClientSocket(UDP *udp_client,char *server_ip,int port);
extern int sendUDPClientData(UDP *udp_client,void *data,size_t data_len);
extern int closeUDPClientSocket(UDP *udp_client);

//udp server prototypes
extern int openUDPServerSocket(UDP *udp_server,int port);
extern int receiveUDPServerData(UDP *udp_server,void *data,size_t data_len);
extern int closeUDPServerSocket(UDP *udp_server);

#endif /*UDP_COMMUNCATION_H__*/

