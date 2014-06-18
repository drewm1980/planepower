#include<string.h> //memset
#include<stdlib.h> //exit
#include <unistd.h> // close
#include "udp_communication.h"

#ifndef DEBUG 
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#endif

/********************************
 * PROTOTYPES PRIVATE
 * ******************************/
static UDP_errCode openUDPSocket(UDP *udp,unsigned int timeout);

/********************************
 * FUNCTIONS
 * ******************************/
// timeout in us
UDP_errCode openUDPClientSocket(UDP *udp_client,const char *server_ip,int port,unsigned int timeout){
	
	#if DEBUG  > 1
		printf("Entering openUDPClientSocket\n");
	#endif
	
	openUDPSocket(udp_client,timeout);
	
    //zero out the structure
	memset((char *) &(udp_client->si_other), 0,udp_client->fd_len);

	udp_client->si_other.sin_family = AF_INET;
	udp_client->si_other.sin_port = htons(port);
     
	// inet_aton converts the Internet host address cp from the standard numbers-and-dots notation into binary data and stores it in the structure that inp points to. inet_aton returns nonzero if the address is valid, zero if not
	if (inet_aton(server_ip , &(udp_client->si_other.sin_addr)) == 0) 
	{
		return UDP_ERR_INET_ATON;
	}
	return UDP_ERR_NONE;
}

UDP_errCode sendUDPClientData(UDP *udp_client,void *data,size_t data_len){
	
	#if DEBUG  > 1
		printf("Entering sendUDPClientData\n");
	#endif
	
	//send the message
	if (sendto(udp_client->fd, (void *)data, data_len , 0 , (struct sockaddr *) &(udp_client->si_other), udp_client->fd_len)==-1)
	{
		return UDP_ERR_SEND;
	}
	return UDP_ERR_NONE;
}

UDP_errCode closeUDPClientSocket(UDP *udp_client){
	
	#if DEBUG  > 1
		printf("Entering closeUDPClientSocket\n");
	#endif
	
	if(close(udp_client->fd)==-1)
		return UDP_ERR_CLOSE_SOCKET; 
	else
		return UDP_ERR_NONE;
}

// timeout in us
static UDP_errCode openUDPSocket(UDP *udp,unsigned int timeout)
{
#if DEBUG  > 1
	printf("Entering openUDPSocket\n");
#endif

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout;
	
	//define fd_len
    udp->fd_len = sizeof(udp->si_other);
    
    //create a UDP socket
	if ( (udp->fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		return UDP_ERR_OPEN_SOCKET;
	} 
	if(setsockopt(udp->fd, SOL_SOCKET, SO_RCVTIMEO,(char*)&tv,sizeof(tv))){
		return UDP_ERR_SET_TIMEOUT;
	}

	return UDP_ERR_NONE;
}



UDP_errCode openUDPServerSocket(UDP *udp_server,int port,unsigned int timeout){
	
	#if DEBUG  > 1
		printf("Entering openUDPServerSocket\n");
	#endif
	
	openUDPSocket(udp_server,timeout);
	
	// zero out the structure
	memset((char *) &(udp_server->si_me), 0, sizeof(udp_server->si_me));

	udp_server->si_me.sin_family = AF_INET;
	udp_server->si_me.sin_port = htons(port);
	udp_server->si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(udp_server->fd , (struct sockaddr*)&(udp_server->si_me), sizeof(udp_server->si_me) ) == -1)
	{
		return UDP_ERR_BIND_SOCKET_PORT;
	}
	return UDP_ERR_NONE;
}

UDP_errCode receiveUDPServerData(UDP *udp_server,void *data,size_t data_len){
	
	#if DEBUG  > 1
		printf("Entering receiveUDPServerData\n");
	#endif
	int recv_len;
	//blocking !!!
	if ((recv_len = recvfrom(udp_server->fd, data, data_len, 0, (struct sockaddr *) &(udp_server->si_other), (socklen_t *)&(udp_server->fd_len))) == -1)
	{
		printf("Problem receiving UDP packet!\n");
		return UDP_ERR_RECV;
	}
	if (recv_len != data_len)
	{
		printf("Wrong number of bytes in received UDP packet!\n");
		printf("Expected %lu bytes, Received %li bytes!\n",data_len,recv_len);
		return UDP_ERR_RECV;
	}
	return UDP_ERR_NONE;
}


UDP_errCode closeUDPServerSocket(UDP *udp_server){
	
	#if DEBUG  > 1
		printf("Entering closeUDPServerSocket\n");
	#endif
	
	if(close(udp_server->fd)==-1)
		return UDP_ERR_CLOSE_SOCKET; 
	else
		return UDP_ERR_NONE;
}

void UDP_err_handler( UDP_errCode err,void (*write_error_ptr)(char *,char *,int))
{
	#if DEBUG  > 1
		printf("Entering UDP_err_handler\n");
	#endif
	
	static char SOURCEFILE[] = "udp_communication.c";
	
	switch( err ) {
		case UDP_ERR_NONE:
			break;
		case  UDP_ERR_INET_ATON:
			write_error_ptr(SOURCEFILE,"failed decoding ip address",err);
			break;
		case UDP_ERR_SEND:
			write_error_ptr(SOURCEFILE,"failed sending UDP data",err);
			break;
		case UDP_ERR_CLOSE_SOCKET:
			write_error_ptr(SOURCEFILE,"failed closing UDP socket",err);
			break;
		case UDP_ERR_OPEN_SOCKET:
			write_error_ptr(SOURCEFILE,"failed opening UDP socket",err);
			break;
		case UDP_ERR_BIND_SOCKET_PORT:
			write_error_ptr(SOURCEFILE,"failed binding port to socket",err);
			break;
		case UDP_ERR_RECV:
			write_error_ptr(SOURCEFILE,"failed receiving UDP data",err);
			break;
		case UDP_ERR_SET_TIMEOUT:
			write_error_ptr(SOURCEFILE,"failed setting UDP timeout on socket",err);
			break;
		case UDP_ERR_UNDEFINED:
			write_error_ptr(SOURCEFILE,"undefined UDP error",err);
			break;
		default: break;
	}
}

