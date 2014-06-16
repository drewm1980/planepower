#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>

typedef struct __attribute__((packed))
{
	unsigned char dstMac[6];
	unsigned char srcMac[6]; 
	unsigned char etherTypeOrSize[2];  // 0101 is a Xerox experimental
} Header;

typedef struct __attribute__((packed))
{
	Header header;
	unsigned char payload[63-sizeof(Header)];
} Packet;

#define IFNAME "eth0"

Packet highwind_raw_packet;
int highwind_raw_socket_fd = -1;


int setup_socket_and_packet(unsigned char dstMac[6])
{
	// Create the socket
    highwind_raw_socket_fd = socket( AF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );
	printf("socket fd is %i\n",highwind_raw_socket_fd);
	if(highwind_raw_socket_fd == -1)
	{ perror( "init: socket" ); close( highwind_raw_socket_fd ); return -1; }
	perror("Status after creating socket");

	// configure to connect to specific interface ?
    struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), IFNAME); // <--- eth0 goes here
	int rc;
	if ((rc = setsockopt(highwind_raw_socket_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr))) < 0)
	{
		perror("Server-setsockopt() error for SO_BINDTODEVICE");
		printf("%s\n", strerror(errno));
		close(highwind_raw_socket_fd);
		exit(-1);
	}
	perror("Status after calling setsockopt");

	// get local mac address, and Store the mac address in the packet
    if ( ioctl( highwind_raw_socket_fd, SIOCGIFHWADDR, &ifr ) < 0 )
    {
        perror( "init: ioctl SIOCGIFHWADDR" );
        close( highwind_raw_socket_fd );
        exit(-1);
    }
	perror("Status after ioctl to get local MAC");
	memset(&highwind_raw_packet, 0, sizeof(highwind_raw_packet));
	memcpy( highwind_raw_packet.header.dstMac, dstMac, 6) ;
    memcpy( highwind_raw_packet.header.srcMac, (unsigned char*)ifr.ifr_hwaddr.sa_data, 6 );
	highwind_raw_packet.header.etherTypeOrSize[0] = 0x01; // Stolen from Xerox!
	highwind_raw_packet.header.etherTypeOrSize[1] = 0x01; // Stolen from Xerox!
	printf("Source MAC: ");
	printf("%x",highwind_raw_packet.header.srcMac[0]);
	printf("%x",highwind_raw_packet.header.srcMac[1]);
	printf("%x",highwind_raw_packet.header.srcMac[2]);
	printf("%x",highwind_raw_packet.header.srcMac[3]);
	printf("%x",highwind_raw_packet.header.srcMac[4]);
	printf("%x\n",highwind_raw_packet.header.srcMac[5]); // Worked
	printf("Dest MAC: ");
	printf("%x",highwind_raw_packet.header.dstMac[0]);
	printf("%x",highwind_raw_packet.header.dstMac[1]);
	printf("%x",highwind_raw_packet.header.dstMac[2]);
	printf("%x",highwind_raw_packet.header.dstMac[3]);
	printf("%x",highwind_raw_packet.header.dstMac[4]);
	printf("%x\n",highwind_raw_packet.header.dstMac[5]); // Worked
	
	// get interface index
    if ( ioctl( highwind_raw_socket_fd, SIOCGIFINDEX, &ifr ) < 0 )
	{ perror( "init: ioctl" ); close( highwind_raw_socket_fd ); return -1; }

	printf("Device index is %i\n",ifr.ifr_ifindex);
	perror("Status after ioctl to get device index");

	// Bind the socket
	struct sockaddr_ll addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sll_family   = AF_PACKET;
    addr.sll_protocol = 0;
    addr.sll_ifindex  = ifr.ifr_ifindex;
	addr.sll_halen = 6;
	memcpy(addr.sll_addr,highwind_raw_packet.header.srcMac,6);
    if ( bind( highwind_raw_socket_fd, (const struct sockaddr*)&addr, sizeof( addr ) ) < 0 )
    {
        perror( "init: bind fails" );
        close( highwind_raw_socket_fd );
        return -1;
    }
	return 0;
}

void send_packet()
{
	perror("Entering send_packet"); 
	int bytes = write( highwind_raw_socket_fd, 
			&highwind_raw_packet, 
			sizeof(highwind_raw_packet));
	printf("bytes written: %i\n",bytes);
	perror("Status after write");
}

int main()
{
	unsigned char dstMac[6] = {0xc8,0xa0,0x30,0xc5,0xb9,0xbb};
	setup_socket_and_packet(dstMac);
	
	// Copy in our payload
	highwind_raw_packet.payload[0] = 1;
	highwind_raw_packet.payload[1] = 2;
	highwind_raw_packet.payload[2] = 3;
	highwind_raw_packet.payload[3] = 4;

	printf("socket fd is %i\n",highwind_raw_socket_fd);

	//while(1);

	// Send!
	send_packet();
}
