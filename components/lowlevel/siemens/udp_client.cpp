#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <string>
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	cout << "Starting TCP client example slightly modified from wikipedia..." << endl;

	int sock;
	struct sockaddr_in sa;
	int bytes_sent;

	//create an internet, datagram, socket using UDP
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (-1 == sock) /* if socket failed to initialize, exit */
	{
		printf("Error Creating Socket");
		exit(EXIT_FAILURE);
	}
	cout << "Socket created..." << endl;

	//Zero out socket address
	memset(&sa, 0, sizeof sa);

	//The address is ipv4
	sa.sin_family = AF_INET;

	//ip_v4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value
	sa.sin_addr.s_addr = inet_addr("192.168.0.1");

	//sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654
	uint16_t port=2000;
	sa.sin_port = htons(port);

	char buffer[200];
	strcpy(buffer, "hello world!");
	//sendto(int socket, char data, int dataLength, flags, destinationAddress, int destinationStructureLength)
	bytes_sent = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr*)&sa, sizeof sa);
	if (bytes_sent < 0) {
		printf("Error sending packet: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	cout << "Sendto succeeded..." << endl;

	close(sock); /* close the socket */
	cout << "Closed socket..." << endl;
	return 0;
}

