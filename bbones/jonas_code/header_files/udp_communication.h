/*
 * AUTHOR: Jonas Van Pelt
 */

#ifndef UDP_COMMUNCATION_H_ 
#define UDP_COMMUNCATION_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include<sys/socket.h>
#include<arpa/inet.h>


/********************************
 * GLOBALS
 * ******************************/

// UDP error codes 
enum udp_errCode { UDP_ERR_NONE = 0, UDP_ERR_INET_ATON, UDP_ERR_SEND, UDP_ERR_CLOSE_SOCKET,UDP_ERR_OPEN_SOCKET,UDP_ERR_BIND_SOCKET_PORT,UDP_ERR_RECV,UDP_ERR_SET_TIMEOUT, UDP_ERR_UNDEFINED };

typedef enum udp_errCode UDP_errCode;

typedef struct 
{ 
	struct sockaddr_in si_me;
	struct sockaddr_in si_other;
	int fd;
	int fd_len;
} UDP;


/********************************
 * PROTOTYPES PUBLIC
 * ******************************/
//udp client prototypes
extern UDP_errCode openUDPClientSocket(UDP *udp_client,const char *server_ip,int port,unsigned int timeout);
extern UDP_errCode sendUDPClientData(UDP *udp_client,void *data,size_t data_len);
extern UDP_errCode closeUDPClientSocket(UDP *udp_client);

//udp server prototypes
extern UDP_errCode openUDPServerSocket(UDP *udp_server,int port,unsigned int timeout);
extern UDP_errCode receiveUDPServerData(UDP *udp_server,void *data,size_t data_len);
extern UDP_errCode closeUDPServerSocket(UDP *udp_server);

void UDP_err_handler( UDP_errCode err,void (*write_error_ptr)(char *,char *,int));

#ifdef __cplusplus
}
#endif

#endif /*UDP_COMMUNCATION_H__*/

