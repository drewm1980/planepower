#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#include <string>
#include <iostream>
using namespace std;


int main(void)
{
	cout << "Starting UDP server example slightly modified from wikipedia..." << endl;

	int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in sa; 
	socklen_t fromlen;
	cout << "Socket created..." << endl;

	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	uint16_t port=2000;
	cout << "UDP server will listen on port " << port << "..." << endl;
	sa.sin_port = htons(port);
	fromlen = sizeof(sa);

	if (-1 == bind(sock,(struct sockaddr *)&sa, sizeof(sa)))
	{
		perror("error bind failed");
		close(sock);
		exit(EXIT_FAILURE);
	} 
	cout << "Bind succeeded..." << endl;

	for (;;) 
	{
		printf ("Running recvfrom....\n");
		char buffer[1024];
		ssize_t recsize;
		recsize = recvfrom(sock, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&sa, &fromlen);
		if (recsize < 0) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		cout << "Returned from recvfrom..." << endl;
		printf("recsize: %d\n ", (int)recsize);
		sleep(1);
		printf("datagram: %.*s\n", (int)recsize, buffer);
	}
}
