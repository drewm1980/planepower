/*
        demo-udp-03: udp-recv: a simple udp server
	receive udp messages

        usage:  udp-recv

        Paul Krzyzanowski
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "port.h"

#define BUFSIZE 2048

int
main(int argc, char **argv)
{
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen = sizeof(remaddr);		/* length of addresses */
	int recvlen;			/* # bytes received */
	int fd;				/* our socket */
	unsigned char buf[BUFSIZE];	/* receive buffer */


	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
        struct timeval startTime, endTime;
        float neededSeconds;
        int i = 0;
        int msgCounter = 0;
	/* now loop, receiving data and printing what we received */
	for (i ; i < 1 ; i = i) {
		// printf("waiting on port %d\n", SERVICE_PORT);
		recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		// printf("received %d bytes\n", recvlen);
                if (buf[0] == 's') gettimeofday(&startTime, 0);
                if (buf[0] == 'e') {
                  gettimeofday(&endTime,0);
                  i = 1;
                }
		if (recvlen > 0) {
			buf[recvlen] = 0;
			// printf("received message: \"%s\"\n", buf);
                        msgCounter ++;
                        if (msgCounter % 1000 == 0) {
                          gettimeofday(&endTime,0);
                          neededSeconds = endTime.tv_sec - startTime.tv_sec + 0.000001 * (endTime.tv_usec - startTime.tv_usec);
                          printf("%i\t%f\n", msgCounter, neededSeconds);
                        }
		}
	}
        neededSeconds = endTime.tv_sec - startTime.tv_sec + 0.000001 * (endTime.tv_usec - startTime.tv_usec);
        printf ("\nMessages received: %i\n", msgCounter);
        printf ("Average time for receiving one package: %f\n", neededSeconds/msgCounter);

	/* never exits */
}
