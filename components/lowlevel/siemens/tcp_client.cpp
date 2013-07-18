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
	cout << "Starting TCP client example slightly modified from wikipedia..." << endl;

	struct sockaddr_in stSockAddr;
	int Res;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (-1 == SocketFD)
	{
		perror("cannot create socket");
		exit(EXIT_FAILURE);
	}
	cout << "Socket created..." << endl;

	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	uint16_t port=2000;
	stSockAddr.sin_port = htons(port);
	cout << "Port on remote server that will connect to is " << port << endl;
	const char server_ip[] = "192.168.0.1";
	Res = inet_pton(AF_INET, server_ip, &stSockAddr.sin_addr);
	if (0 > Res)
	{
		perror("error: first parameter is not a valid address family");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
	else if (0 == Res)
	{
		perror("char string (second parameter does not contain valid ipaddress)");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
	cout << "IP address string made sense..." << endl;

	if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("Connect failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
	cout << "Connect succeeded..." << endl;

	/* perform read write operations ... */
	//cout << "Attempting to read 4 bytes..." << endl;
	//unsigned char buff[4];
	//ssize_t bytes_read = read(ConnectFD,buff,sizeof(buff));
	//if(bytes_read==0)
		//cout << "No bytes read!" << endl;
	//else
		//cout << bytes_read << " bytes read!" << endl;

	cout << "Shutting everything down..." << endl;

	(void) shutdown(SocketFD, SHUT_RDWR);

	close(SocketFD);


	return EXIT_SUCCESS;
}
