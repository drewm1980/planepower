#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0
#include "udp_communication.hpp"

#include <iostream>

using namespace std;

int openUDPClientSocket(UDP *udp_client,char *server_ip,int port)
{
	
	openUDPSocket(udp_client);
	
    //zero out the structure
	memset((char *) &(udp_client->si_other), 0,udp_client->fd_len);

	udp_client->si_other.sin_family = AF_INET;
	udp_client->si_other.sin_port = htons(port);
     
	// inet_aton converts the Internet host address cp from the standard numbers-and-dots notation into binary data and stores it in the structure that inp points to. inet_aton returns nonzero if the address is valid, zero if not
	if (inet_aton(server_ip , &(udp_client->si_other.sin_addr)) == 0) 
	{
		cout << "inet_anon() failed!" << endl;
		//error_write(FILENAME,"openUDPClientSocket()","inet_aton() failed");
		return -1;
	}
	return 0;
}

int sendUDPClientData(UDP *udp_client,void *data,size_t data_len){
	//send the message
	if (sendto(udp_client->fd, (void *)data, data_len , 0 , (struct sockaddr *) &(udp_client->si_other), udp_client->fd_len)==-1)
	{
		cout << "sendto failed" << endl;
		//error_write(FILENAME,"sendUDPClientData()","sendto() failed");
		return -1;
	}
	return 0;
}

int closeUDPClientSocket(UDP *udp_client){
	return close(udp_client->fd); 
}

int openUDPSocket(UDP *udp){
	
	//define fd_len
    udp->fd_len = sizeof(udp->si_other);
    
    //create a UDP socket
	if ( (udp->fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		cout << "opening socket failed!" << endl;
		//error_write(FILENAME,"openUDPSocket()","opening socket failed");
		return -1;
	} 

	return 0;
}



int openUDPServerSocket(UDP *udp_server,int port){
	
	openUDPSocket(udp_server);
	
	// zero out the structure
	memset((char *) &(udp_server->si_me), 0, sizeof(udp_server->si_me));

	udp_server->si_me.sin_family = AF_INET;
	udp_server->si_me.sin_port = htons(port);
	udp_server->si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(udp_server->fd , (struct sockaddr*)&(udp_server->si_me), sizeof(udp_server->si_me) ) == -1)
	{
		cout << "bindingn socket to port failed!" << endl;
		//error_write(FILENAME,"openUDPServerSocket()","binding socket to port failed");
		return -1;
	}
	return 0;
}

int receiveUDPServerData(UDP *udp_server,void *data,size_t data_len){
	int recv_len;
	//blocking !!!
	recv_len = recvfrom(udp_server->fd, 
			data,
			data_len, 
			0, 
			(struct sockaddr *) &(udp_server->si_other), 
			(unsigned int *) &(udp_server->fd_len));
	if (recv_len==-1)
	{
		cout << "recvfrom() failed!" << endl;
		//error_write(FILENAME,"receiveUDPServerData()","recvfrom() failed");
		return -1;
	}else{
		//eventually send something back to sender using si_other
	}
	return 0;
}


int closeUDPServerSocket(UDP *udp_server){
	return close(udp_server->fd);
}


