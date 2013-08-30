#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>

using namespace std;
int main(int argc, char **argv) 
{
	cout << "Starting Profinet Test Code" << endl;

	// Open a new network socket
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(-1 == SocketFD)
	{
		perror("Could not create socket");
		exit(EXIT_FAILURE);
	}

	// Intialize structure to configure the socket (mainly, bind it to a port)
	struct sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(stSockAddr));
	stSockAddr.sin_family = AF_INET; // for ipv4
	stSockAddr.sin_port = htons(1100); // host to network byte order for unsigned short int
	stSockAddr.sin_addr.s_addr = INADDR_ANY;

	if(-1 == bind(SocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("error bind failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	if(-1 == listen(SocketFD, 10))
	{
		perror("error listen failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	for(;;)
	{
		int ConnectFD = accept(SocketFD, NULL, NULL);

		if(0 > ConnectFD)
		{
			perror("error accept failed");
			close(SocketFD);
			exit(EXIT_FAILURE);
		}

		/* perform read write operations ... 
		   read(ConnectFD,buff,size)*/

		if (-1 == shutdown(ConnectFD, SHUT_RDWR))
		{
			perror("can not shutdown socket");
			close(ConnectFD);
			close(SocketFD);
			exit(EXIT_FAILURE);
		}
		close(ConnectFD);
	}

	close(SocketFD);
	return EXIT_SUCCESS;  
}
