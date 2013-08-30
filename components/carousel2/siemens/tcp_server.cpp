/* Server code in C */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <iostream>
using namespace std;

int main(void)
{
	cout << "Starting TCP server example slightly modified from wikipedia..." << endl;
	struct sockaddr_in stSockAddr;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(-1 == SocketFD)
	{
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}
	cout << "Socket created..." << endl;

	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	uint16_t port=2000;
	stSockAddr.sin_port = htons(port);
	cout << "Port we will listen on is " << port << endl;
	stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(-1 == bind(SocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("error bind failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
	cout << "Bind suceeded..." << endl;

	if(-1 == listen(SocketFD, 10))
	{
		perror("error listen failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
	cout << "Listen succeeded..." << endl;

	for(;;)
	{
		int ConnectFD = accept(SocketFD, NULL, NULL);

		if(0 > ConnectFD)
		{
			perror("error accept failed");
			close(SocketFD);
			exit(EXIT_FAILURE);
		}
		cout << "Accept succeeded..." << endl;

		//perform read write operations ... 
		cout << "Attempting to read 4 bytes..." << endl;
		unsigned char buff[4];
		ssize_t bytes_read = read(ConnectFD,buff,sizeof(buff));
		if(bytes_read==0)
			cout << "No bytes read!" << endl;
		else
			cout << bytes_read << " bytes read!" << endl;

		if (-1 == shutdown(ConnectFD, SHUT_RDWR))
		{
			perror("can not shutdown socket");
			close(ConnectFD);
			close(SocketFD);
			exit(EXIT_FAILURE);
		}
		cout << "Connection was shutdown normally.  Further receptions and transmissions disabled..." << endl;
		close(ConnectFD);
		cout << "Connection file closed..." << endl;
	}

	close(SocketFD);
	cout << "Socket file closed..." << endl;
	return EXIT_SUCCESS;  
}
