#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define RECEIVE_BUFFER_SIZE		64
#define TRANSMIT_BUFFER_SIZE	64

#define TAG_CMD        			0xff

#define CMD_SET_MOTOR_REFS		0x40
#define CMD_GET_SENSOR_DATA		0x41

typedef struct TIMUSensorData
{
	short XGyro;
	short YGyro;
	short ZGyro;
	
	short XAccl;
	short YAccl;
	short ZAccl;
	
	short XTemp;
	short YTemp;
	short ZTemp;	
} IMUSensorData;

IMUSensorData g_sIMUSensorData;

unsigned char g_ucReceiveBuffer[ RECEIVE_BUFFER_SIZE ];
unsigned char g_ucTransmitBuffer[ TRANSMIT_BUFFER_SIZE ];

unsigned int g_uiNumOfBytesToBeReceived = 0;
unsigned int g_uiNumOfBytesToBeTransmitted = 0;

int g_TCPSocket;

void Die(char *mess) { perror(mess); exit( 1 ); }

int EthernetTransmitReceive( void );

int SendMotorReferences( short m1_ref, short m2_ref, short m3_ref )
{
	//
	// Prepare the request message
	//
	g_ucTransmitBuffer[ 0 ] = TAG_CMD;
	g_ucTransmitBuffer[ 1 ] = 0x0A;
	g_ucTransmitBuffer[ 2 ] = CMD_SET_MOTOR_REFS;

	g_ucTransmitBuffer[ 3 ] = m1_ref >> 8;
	g_ucTransmitBuffer[ 4 ] = m1_ref;
	g_ucTransmitBuffer[ 5 ] = m2_ref >> 8;
	g_ucTransmitBuffer[ 6 ] = m2_ref;
	g_ucTransmitBuffer[ 7 ] = m3_ref >> 8;
	g_ucTransmitBuffer[ 8 ] = m3_ref;

	g_uiNumOfBytesToBeTransmitted = g_ucTransmitBuffer[ 1 ];
	g_uiNumOfBytesToBeReceived = 0x04;

	//
	// Transmit the request and wait for the response
	//
	if ( EthernetTransmitReceive() != 0 )
	{
		printf( "Sending of the motor references failed\n" );		
		return 1;
	}

	return 0;
}

int ReceiveSensorData( void )
{
	//
	// Prepare the request message
	//
	g_ucTransmitBuffer[ 0 ] = TAG_CMD;
	g_ucTransmitBuffer[ 1 ] = 0x04;
	g_ucTransmitBuffer[ 2 ] = CMD_GET_SENSOR_DATA;

	g_uiNumOfBytesToBeTransmitted = g_ucTransmitBuffer[ 1 ];
	g_uiNumOfBytesToBeReceived = 0x16;

	//
	// Transmit the request and wait for the response
	//
	if ( EthernetTransmitReceive() != 0 )
	{
		printf( "Sending the command for receiving the sensor data failed\n" );		
		return 1;
	}

	//
	// Process the response
	//

	if ( g_ucReceiveBuffer[ 2 ] !=  CMD_GET_SENSOR_DATA )
	{
		printf( "Bad command response!\n" );
	} 
	
	g_sIMUSensorData.XGyro = ( g_ucReceiveBuffer[ 3  ] << 8 ) + g_ucReceiveBuffer[ 4  ];
	g_sIMUSensorData.YGyro = ( g_ucReceiveBuffer[ 5  ] << 8 ) + g_ucReceiveBuffer[ 6  ];
	g_sIMUSensorData.ZGyro = ( g_ucReceiveBuffer[ 7  ] << 8 ) + g_ucReceiveBuffer[ 8  ];

	g_sIMUSensorData.XAccl = ( g_ucReceiveBuffer[ 9  ] << 8 ) + g_ucReceiveBuffer[ 10 ];
	g_sIMUSensorData.YAccl = ( g_ucReceiveBuffer[ 11 ] << 8 ) + g_ucReceiveBuffer[ 12 ];
	g_sIMUSensorData.ZAccl = ( g_ucReceiveBuffer[ 13 ] << 8 ) + g_ucReceiveBuffer[ 14 ];

	g_sIMUSensorData.XTemp = ( g_ucReceiveBuffer[ 15 ] << 8 ) + g_ucReceiveBuffer[ 16 ];
	g_sIMUSensorData.YTemp = ( g_ucReceiveBuffer[ 17 ] << 8 ) + g_ucReceiveBuffer[ 18 ];
	g_sIMUSensorData.ZTemp = ( g_ucReceiveBuffer[ 19 ] << 8 ) + g_ucReceiveBuffer[ 20 ];

	return 0;
}

int EthernetTransmitReceive( void )
{
	static int i, j;
	static unsigned long index, sum;

	// first calculate the checksum...
	for( index = 0, sum = 0; index < ( g_uiNumOfBytesToBeTransmitted - 1 ); index++ )
    {
        sum -= g_ucTransmitBuffer[ index ];
    }
    g_ucTransmitBuffer[ g_uiNumOfBytesToBeTransmitted - 1 ] = sum;

	// then send...
	if ( send( g_TCPSocket, g_ucTransmitBuffer, g_uiNumOfBytesToBeTransmitted, 0 )
		!= g_uiNumOfBytesToBeTransmitted )
	{
		printf( "TCP sending failed\n" );		
		return 1;
	}

	// after that receive...
	for ( i = 0; i < g_uiNumOfBytesToBeReceived; )
	{
		j = 0;

		if ( ( j = recv( g_TCPSocket, g_ucReceiveBuffer, g_uiNumOfBytesToBeReceived, 0 ) ) < 1 )
		{
			return 1;
			printf( "Failed to receive bytes from server\n" );
		}

		i += j;
	}

	// calculate the checksum...
	for( index = 0, sum = 0; index < g_uiNumOfBytesToBeReceived; index++ )
	{
		sum += g_ucReceiveBuffer[ index ];
	}
	if ( ( sum & 0xFF ) != 0 )
	{
		printf( "Checksum of received data package is bad\n" );		
		return 1;
	}
	
	return 0;
}

int main( int argc, char *argv[] )
{
	struct sockaddr_in echoserver;
	int i, status;
	FILE* outputfile;
	short counter = 0;

	if ( argc != 3 )
	{
		fprintf(stderr, "USAGE: simpleclient <server_ip> <port>\n");

		exit(1);
	}

	// Create the TCP socket
	if ( ( g_TCPSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
	{
		Die("Failed to create socket");
	}

	// Construct the server sockaddr_in structure
	memset( &echoserver, 0, sizeof( echoserver ) );			// Clear struct
	echoserver.sin_family = AF_INET;						// Internet/IP
	echoserver.sin_addr.s_addr = inet_addr( argv[ 1 ] );	// IP address
	echoserver.sin_port = htons( atoi( argv[ 2 ] ) );       // Server port
	// Establish connection
	if ( connect( g_TCPSocket,
		( struct sockaddr * ) &echoserver,
		sizeof( echoserver ) ) < 0 )
	{
		Die("Failed to connect with server");
	}

	outputfile = fopen( "datalog.csv", "w" );

	// End do something quite usefull...
	for ( i = 0; i < 1000; i++ )
	{
		status = ReceiveSensorData();
		if ( status != 0 )
		{ break; }
		
		/*
		printf( "XA: %#06x, YA: %#06x, ZA: %#06x, XG: %#06x, YG: %#06x, ZG: %#06x\n",
			(unsigned short) g_sIMUSensorData.XAccl,
			(unsigned short) g_sIMUSensorData.YAccl,
			(unsigned short) g_sIMUSensorData.ZAccl,
			
			(unsigned short) g_sIMUSensorData.XGyro,
			(unsigned short) g_sIMUSensorData.YGyro,
			(unsigned short) g_sIMUSensorData.ZGyro

			//(unsigned short) g_sIMUSensorData.XTemp,
			//(unsigned short) g_sIMUSensorData.YTemp,
			//(unsigned short) g_sIMUSensorData.ZTemp
		);
		*/
		
		/*
		fprintf( outputfile,
			"%d, %d, %d, %d, %d, %d\n",

			g_sIMUSensorData.XAccl,
			g_sIMUSensorData.YAccl,
			g_sIMUSensorData.ZAccl,
			
			g_sIMUSensorData.XGyro,
			g_sIMUSensorData.YGyro,
			g_sIMUSensorData.ZGyro

			//(unsigned short) g_sIMUSensorData.XTemp,
			//(unsigned short) g_sIMUSensorData.YTemp,
			//(unsigned short) g_sIMUSensorData.ZTemp
		);
		*/

		counter += 100;
		SendMotorReferences( counter, counter << 1, counter << 2 );
				
		usleep( 10000 );
	}

	// Wrapup
	if ( status == 0 )
		fprintf( stdout, "Application sucessfully finished.... \n" );

	close( g_TCPSocket );
	fclose( outputfile );

	exit( 0 );
}
