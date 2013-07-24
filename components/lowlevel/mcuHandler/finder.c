//*****************************************************************************
//
// finder.c - Program to locate Ethernet-based Stellaris board on the network.
// 			  Linux ONLY Version
// 			  Milan Vukov, milan.vukov@esat.kuleuven.be
//
//*****************************************************************************
//
// Copyright (c) 2009-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 6852 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

#include <pthread.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <sys/types.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>

//*****************************************************************************
//
// These defines are used to describe the device locator protocol.
//
//*****************************************************************************
#define TAG_CMD						0xff
#define TAG_STATUS					0xfe
#define CMD_DISCOVER_TARGET			0x02
#define RESP_ID_TARGET_PLANEPOWER	0x00

//*****************************************************************************
//
// A structure that contains the details of a local network interface.
//
//*****************************************************************************
typedef struct
{
    //
    // The UDP socket on this network interface.
    //
    int iSocket;

    //
    // The name of this network interface.
    //
    char pcIFName[32];
}
tSocketData;

//*****************************************************************************
//
// A structure that contains the details of a board found on the network.
//
//*****************************************************************************
typedef struct
{
    //
    // The type of board, typically used by motor control boards.
    //
    unsigned char ucBoardType;

    //
    // The board ID.
    //
    unsigned char ucBoardID;

    //
    // The MAC address of the board.
    //
    unsigned char pucMACArray[6];

    //
    // The IP address of the board.
    //
    unsigned long ulIPAddr;

    //
    // The IP address of the client presently connected to the board.
    //
    unsigned long ulClientIPAddr;

    //
    // The version of the firmware on the board.
    //
    unsigned long ulVersion;

    //
    // The title of the application on the board.
    //
    char pcAppTitle[64];
}
tBoardData;

//*****************************************************************************
//
// An array of active network interfaces.  There is room in this array for 16
// network interfaces, which should be more than enough for most any machine.
//
//*****************************************************************************
tSocketData g_sSockets[16];

//*****************************************************************************
//
// The number of sockets in g_sSockets that are valid.
//
//*****************************************************************************
unsigned long g_ulNumSockets;

//*****************************************************************************
//
// An array of data describing the boards found on the network(s).  There is
// room in this array for 256 board, which should be enough for typical network
// setups.
//
//*****************************************************************************
tBoardData g_psBoardData[256];

//*****************************************************************************
//
// The number of board descriptions in g_psBoardData.
//
//*****************************************************************************
unsigned long g_ulNumBoards;

//*****************************************************************************
//
// A boolean that is true when the network should be rescanned for devices.
//
//*****************************************************************************
volatile unsigned long g_ulRefresh = 1;

//*****************************************************************************
//
// This function creates a UDP socket for each active network interface.
//
//*****************************************************************************
void
CreateIFSockets(void)
{
    int iSocket, iIdx, iNumInterfaces;
    struct sockaddr_in *pAddr;
    unsigned long ulTemp;
    char pcBuffer[1024];
    struct ifconf sIFC;
    struct ifreq *pIFR;

    //
    // Initialize the number of sockets to zero.
    //
    g_ulNumSockets = 0;

    //
    // Create a generic UDP socket to use to query the network interfaces.
    //
    if((iSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        return;
    }

    //
    // Get the list of network interfaces.
    //
    sIFC.ifc_len = sizeof(pcBuffer);
    sIFC.ifc_buf = pcBuffer;
    if(ioctl(iSocket, SIOCGIFCONF, &sIFC) < 0)
    {
        close(iSocket);
        return;
    }

    //
    // Close the socket.
    //
    close(iSocket);

    //
    // Get the size and pointer to the network interface list.
    //
    iNumInterfaces = sIFC.ifc_len / sizeof(struct ifreq);
    pIFR = sIFC.ifc_req;

    //
    // Loop through the network interfaces.
    //
    for(iIdx = 0; iIdx < iNumInterfaces; iIdx++)
    {
        //
        // Get the address pointer for this network interface.
        //
        pAddr = (struct sockaddr_in *)&(pIFR[iIdx].ifr_addr);

        //
        // Skip this network interface if it is not configured (the network
        // address is zero) or it is the loopback interface (the network
        // address is 127.0.0.1).
        //
        if((pAddr->sin_addr.s_addr == 0x00000000) ||
           (pAddr->sin_addr.s_addr == 0x0100007f))
        {
            continue;
        }

        //
        // Create a socket.
        //
        if((iSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            continue;
        }

        //
        // Bind this socket to the IP address of this interface.
        //
        pAddr->sin_port = htons(8003);
        if(bind(iSocket, (struct sockaddr *)pAddr,
                sizeof(struct sockaddr_in)) < 0)
        {
            close(iSocket);
            continue;
        }

        //
        // Put this socket into non-blocking mode.
        //
        ulTemp = 0;
        if(ioctl(iSocket, FIONBIO, &ulTemp) < 0)
        {
            close(iSocket);
            continue;
        }

        //
        // Enable broadcast transmission on this socket.
        //
        ulTemp = 1;
        if(setsockopt(iSocket, SOL_SOCKET, SO_BROADCAST, (char *)&ulTemp,
                      sizeof(ulTemp)) < 0)
        {
            close(iSocket);
            continue;
        }

        //
        // Save this socket.
        //
        strncpy(g_sSockets[g_ulNumSockets].pcIFName, pIFR[iIdx].ifr_name,
                sizeof(g_sSockets[g_ulNumSockets].pcIFName));
        g_sSockets[g_ulNumSockets].pcIFName[sizeof(g_sSockets[0].pcIFName) -
                                            1] = 0;
        g_sSockets[g_ulNumSockets++].iSocket = iSocket;

        //
        // Ignore the remaining network interfaces if the socket array is now
        // full.
        //
        if(g_ulNumSockets == (sizeof(g_sSockets) / sizeof(g_sSockets[0])))
        {
            break;
        }
    }
}

//*****************************************************************************
//
// This function sends device discover messages on the UDP sockets for each
// active network interface.
//
//*****************************************************************************
void
SendDiscovers(void)
{
    struct sockaddr_in sAddr;
    char pcBuffer[4];
    int iIdx;

    //
    // Loop over the UDP sockets.
    //
    for(iIdx = 0; iIdx < g_ulNumSockets; iIdx++)
    {
        //
        // Construct the discover message.
        //
        pcBuffer[0] = TAG_CMD;
        pcBuffer[1] = 4;
        pcBuffer[2] = CMD_DISCOVER_TARGET;
        pcBuffer[3] = (0 - TAG_CMD - 4 - CMD_DISCOVER_TARGET) & 0xff;

        //
        // Set the address for this message.
        //
        sAddr.sin_family = AF_INET;
        sAddr.sin_addr.s_addr = INADDR_BROADCAST;
        sAddr.sin_port = htons(8003);

        //
        // Send the discover message on this socket.
        //
        sendto(g_sSockets[iIdx].iSocket, pcBuffer, sizeof(pcBuffer), 0,
               (struct sockaddr *)&sAddr, sizeof(sAddr));
    }
}

//*****************************************************************************
//
// This function waits for response messages on the UDP sockets.
//
//*****************************************************************************
void
ReadResponses(void)
{
    unsigned char pucBuffer[256];
    struct sockaddr_in sAddr;
    struct timeval sDelay;
    int iIdx, iMax, iLoop;
    socklen_t sLen;
    fd_set sRead;

    struct sockaddr_in *pAddr;
    struct sockaddr *pMAC;
    struct arpreq sARP;

    //
    // Initially there are no boards.
    //
    g_ulNumBoards = 0;

    //
    // Loop while there is still room in the board data array.
    //
    while(g_ulNumBoards != (sizeof(g_psBoardData) / sizeof(g_psBoardData[0])))
    {
        //
        // Set the delay for the select() call based on the number of boards.
        // Wait for 5 seconds for the first response, and after a response is
        // received only wait for 2 seconds for the remaining responses.
        //
        if(g_ulNumBoards == 0)
        {
            sDelay.tv_sec = 5;
            sDelay.tv_usec = 0;
        }
        else
        {
            sDelay.tv_sec = 1;
            sDelay.tv_usec = 0;
        }

        //
        // Clear the read socket set.
        //
        FD_ZERO(&sRead);

        //
        // Loop over all the sockets to set the appropriate bit in the read
        // socket set and find the maximum socket file descriptor number.
        //
        iMax = 0;
        for(iIdx = 0; iIdx < g_ulNumSockets; iIdx++)
        {
            //
            // Set this file descriptor in the read socket set.
            //
            FD_SET(g_sSockets[iIdx].iSocket, &sRead);

            //
            // Save this as the maximum file descriptor number if it is larger
            // than the previous maximum.
            //
            if(g_sSockets[iIdx].iSocket > iMax)
            {
                iMax = g_sSockets[iIdx].iSocket;
            }
        }

        //
        // Wait until one of the sockets has data to be read or the timeout
        // expires.
        //
        iIdx = select(iMax + 1, &sRead, 0, 0, &sDelay);

        //
        // Retry if there was a select failure.
        //
        if(iIdx < 0)
        {
            continue;
        }

        //
        // Break out of the loop if the timeout expired.
        //
        if(iIdx == 0)
        {
            break;
        }

        //
        // Loop through the sockets.
        //
        for(iIdx = 0; iIdx < g_ulNumSockets; iIdx++)
        {
            //
            // Skip this socket if it does not have data waiting to be read.
            //
            if(!FD_ISSET(g_sSockets[iIdx].iSocket, &sRead))
            {
                continue;
            }

            //
            // Read the datagram from this socket.
            //
            sLen = sizeof(sAddr);
            if(recvfrom(g_sSockets[iIdx].iSocket, (char *)pucBuffer,
                        sizeof(pucBuffer), 0, (struct sockaddr *)&sAddr,
                        &sLen) < 4)
            {
                continue;
            }

            //
            // Ignore this datagram if the first three bytes are not as
            // expected.
            //
            if((pucBuffer[0] != TAG_STATUS) || (pucBuffer[1] < 4) ||
               (pucBuffer[2] != CMD_DISCOVER_TARGET))
            {
                continue;
            }

            //
            // Ignore this datagram if the checksum is not correct.
            //
            for(iLoop = 0, iMax = 0; iLoop < pucBuffer[1]; iLoop++)
            {
                iMax -= pucBuffer[iLoop];
            }
            if((iMax & 0xff) != 0)
            {
                continue;
            }

            //
            // Loop through the boards that have already been found and ignore
            // this datagram if it is a duplicate.
            //
            for(iLoop = 0; iLoop < g_ulNumBoards; iLoop++)
            {
                if(g_psBoardData[iLoop].ulIPAddr == sAddr.sin_addr.s_addr)
                {
                    break;
                }
            }
            if(iLoop != g_ulNumBoards)
            {
                continue;
            }

            //
            // Clear the next board data entry.
            //
            memset(&(g_psBoardData[g_ulNumBoards]), 0,
                   sizeof(g_psBoardData[0]));

            //
            // Save the IP address of this board.
            //
            g_psBoardData[g_ulNumBoards].ulIPAddr = sAddr.sin_addr.s_addr;

            //
            // If the board type was supplied in the response, then save it.
            //
            if(pucBuffer[1] > 4)
            {
                g_psBoardData[g_ulNumBoards].ucBoardType = pucBuffer[3];
            }

            //
            // If the board ID was supplied in the response, then save it.
            //
            if(pucBuffer[1] > 5)
            {
                g_psBoardData[g_ulNumBoards].ucBoardID = pucBuffer[4];
            }

            //
            // If the client IP was supplied in the repsonse, then save it.
            //
            if(pucBuffer[1] > 9)
            {
                memcpy(&(g_psBoardData[g_ulNumBoards].ulClientIPAddr),
                       pucBuffer + 5, 4);
            }

            //
            // If the MAC address was supplied in the response, then save it.
            //
            if(pucBuffer[1] > 15)
            {
                memcpy(g_psBoardData[g_ulNumBoards].pucMACArray, pucBuffer + 9,
                       6);
            }
            else
            {
                //
                // Query the ARP table for the MAC address of the board.  It is
                // possible in some cases that the MAC address does not appear
                // in the table.
                //
                memset(&sARP, 0, sizeof(sARP));
                pAddr = (struct sockaddr_in *)&(sARP.arp_ha);
                pAddr->sin_family = ARPHRD_ETHER;
                pAddr = (struct sockaddr_in *)&(sARP.arp_pa);
                pAddr->sin_family = AF_INET;
                pAddr->sin_addr.s_addr = sAddr.sin_addr.s_addr;
                sARP.arp_flags = ATF_PUBL;
                strncpy(sARP.arp_dev, g_sSockets[iIdx].pcIFName,
                        sizeof(sARP.arp_dev));
                if(strchr(sARP.arp_dev, ':') != 0)
                {
                    *strchr(sARP.arp_dev, ':') = 0;
                }
                if(ioctl(g_sSockets[iIdx].iSocket, SIOCGARP,
                         (char *)&sARP) == 0)
                {
                    pMAC = (struct sockaddr *)&(sARP.arp_ha);
                    memcpy(g_psBoardData[g_ulNumBoards].pucMACArray,
                           pMAC->sa_data, 6);
                }
            }

            //
            // If the firmware version was supplied in the response, then save
            // it.
            //
            if(pucBuffer[1] > 19)
            {
                memcpy(&(g_psBoardData[g_ulNumBoards].ulVersion),
                       pucBuffer + 15, 4);
            }

            //
            // If the application title was supplied in the response, then save
            // it.
            //
            if(pucBuffer[1] > 83)
            {
                memcpy(g_psBoardData[g_ulNumBoards].pcAppTitle, pucBuffer + 19,
                       64);
                g_psBoardData[g_ulNumBoards].pcAppTitle[63] = 0;
            }
            else if(pucBuffer[1] == 10)
            {
                //
                // The application title was not supplied, by the packet size
                // is 10, indicating the older motor control device discovery
                // protocol.  Construct a application title based on the board
                // type.
                //
                if(g_psBoardData[g_ulNumBoards].ucBoardType ==
                   RESP_ID_TARGET_PLANEPOWER)
                {
                    strcpy(g_psBoardData[g_ulNumBoards].pcAppTitle,
                           "RESP_ID_TARGET_PLANEPOWER");
                }
   
                else
                {
                    strcpy(g_psBoardData[g_ulNumBoards].pcAppTitle,
                           "Board type not known...");
                }
            }

            //
            // Increment the count of board that have been found.
            //
            g_ulNumBoards++;

            //
            // Break out of this loop if the board array is now full.
            //
            if(g_ulNumBoards ==
               (sizeof(g_psBoardData) / sizeof(g_psBoardData[0])))
            {
                break;
            }
        }
    }
}

//*****************************************************************************
//
// This function closes the UDP sockets.
//
//*****************************************************************************
void
CloseSockets(void)
{
    int iIdx;

    //
    // Loop through the UDP sockets.
    //
    for(iIdx = 0; iIdx < g_ulNumSockets; iIdx++)
    {
        //
        // Close this socket.
        //
        close(g_sSockets[iIdx].iSocket);
    }

    //
    // There are no sockets open now.
    //
    g_ulNumSockets = 0;
}

//*****************************************************************************
//
// A callback function to display an alert window when the required sockets
// could not be created (likely due to permission problems).
//
//*****************************************************************************
void
RunAsRoot( void )
{
    //
    // Display an alert message.
    //
	printf("Could not create required sockets.  This is either\n");
	printf("a permission problem or another application is\n");
	printf("already using the required ports.");

    //
    // Exit the application.
    //
    exit(1);
}

//*****************************************************************************
//
// A function to print the list of boards.
//
//*****************************************************************************
void
ListBoards( void )
{
    unsigned long ulIdx;

    //
    // Loop through the boards that were found.
    //
    for(ulIdx = 0; ulIdx < g_ulNumBoards; ulIdx++)
    {
        unsigned char* pucMAC = g_psBoardData[ulIdx].pucMACArray;


		printf( "\nIP Address:        %ld.%ld.%ld.%ld\n",
			g_psBoardData[ulIdx].ulIPAddr & 0xff,
			(g_psBoardData[ulIdx].ulIPAddr >> 8) & 0xff,
			(g_psBoardData[ulIdx].ulIPAddr >> 16) & 0xff,
			(g_psBoardData[ulIdx].ulIPAddr >> 24) & 0xff
		);
			
		
		printf( "\nMAC address:       %02x:%02x:%02x:%02x:%02x:%02x\n",
			pucMAC[0], pucMAC[1], pucMAC[2], pucMAC[3], pucMAC[4], pucMAC[5]);

		printf( "\nClient IP Address: %ld.%ld.%ld.%ld\n",
			g_psBoardData[ulIdx].ulClientIPAddr & 0xff,
			(g_psBoardData[ulIdx].ulClientIPAddr >> 8) & 0xff,
			(g_psBoardData[ulIdx].ulClientIPAddr >> 16) & 0xff,
			(g_psBoardData[ulIdx].ulClientIPAddr >> 24) & 0xff);

		printf( "\nApplication title: %s\n", g_psBoardData[ulIdx].pcAppTitle );
    }

    //
    // Awaken the main thread.
    //
    // Fl::awake(pvData);
}

//*****************************************************************************
//
// A worker thread that performs the network queries in the background, so that
// the foreground thread can handle the GUI (allowing it to remain responsive).
//
//*****************************************************************************
void *
WorkerThread(void *pvData)
{
    //
    // Loop forever.  This loop will be explicitly exited as required.
    //
    while(1)
    {
        //
        // Wait until a refresh of the device list is requested.
        //
        while(g_ulRefresh == 0)
        {
            //
            // Awaken the main thread, so that it processes messages in a
            // timely manner.
            //
            // Fl::awake(pvData);

            //
            // Sleep for 100ms.
            //
            usleep(100000);
        }

        //
        // Clear the refresh flag.
        //
        g_ulRefresh = 0;

        //
        // Create the sockets needed to query the network.
        //
        CreateIFSockets();

        //
        // See if the sockets could be created.
        //
        if(g_ulNumSockets == 0)
        {
            //
            // Tell the main thread to display the error message dialog box.
            //
            // Fl::awake(RunAsRoot, 0);
			RunAsRoot();

            //
            // End this thread.
            //
            pthread_exit(0);
        }

        //
        // Send the discover request packets.
        //
        SendDiscovers();

        //
        // Read back the response(s) from the devices on the network.
        //
        ReadResponses();

        //
        // Close the sockets.
        //
        CloseSockets();

        //
        // Tell the main thread to update the board display.
        //
        // Fl::awake(UpdateDisplay, 0);
    }
}

//*****************************************************************************
//
// This program finds Stellaris boards that provide the locator service on the
// Ethernet.
//
//*****************************************************************************
int
main(int argc, char *argv[])
{
    // pthread_t thread;

    // pthread_create(&thread, 0, WorkerThread, 0);

	//
	// Create the sockets needed to query the network.
	//
	printf( "\nCreating the sockets...\n" );
	CreateIFSockets();

	//
	// See if the sockets could be created.
	//
	if(g_ulNumSockets == 0)
	{
		//
		// Tell the main thread to display the error message dialog box.
		//
		RunAsRoot();

		//
		// End this thread.
		//
		exit( 0 );
	}

	//
	// Send the discover request packets.
	//
	printf( "\nSending the discover request packets...\n" );
	SendDiscovers();

	//
	// Read back the response(s) from the devices on the network.
	//
	printf( "\nReading back the responses from the devices on the network...\n" );
	ReadResponses();

	//
	// List the boards and their information
	//
	printf( "\nListing board information...\n" );
	ListBoards();

	//
	// Close the sockets.
	//
	printf( "\nClosing the sockets...\n" );
	CloseSockets();

	return 0;
}
