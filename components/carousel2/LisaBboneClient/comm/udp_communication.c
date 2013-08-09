/*
 * AUTHOR: Jonas Van Pelt
 */

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
extern UDP_errCode openUDPSocket(UDP *udp);


UDP_errCode openUDPClientSocket(UDP *udp_client,const char *server_ip,int port){
	#if DEBUG  > 1
		printf("Entering openUDPClientSocket\n");
	#endif
	
	openUDPSocket(udp_client);
	
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

UDP_errCode openUDPSocket(UDP *udp){
	#if DEBUG  > 1
		printf("Entering openUDPSocket\n");
	#endif
	
	//define fd_len
    udp->fd_len = sizeof(udp->si_other);
    
    //create a UDP socket
	if ( (udp->fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		return UDP_ERR_OPEN_SOCKET;
	} 

	return UDP_ERR_NONE;
}



UDP_errCode openUDPServerSocket(UDP *udp_server,int port){
	#if DEBUG  > 1
		printf("Entering openUDPServerSocket\n");
	#endif
	
	openUDPSocket(udp_server);
	
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
	if ((recv_len = recvfrom(udp_server->fd, data,data_len, 0, (struct sockaddr *) &(udp_server->si_other), (socklen_t *)&(udp_server->fd_len))) == -1)
	{
		return UDP_ERR_RECV;
	}else{
		//eventually send something back to sender using si_other
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


