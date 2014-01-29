//*****************************************************************************
//
// ui_ethernet.c - A simple control interface utilizing Ethernet and the
//                 lwIP TCP/IP stack.
//
// Copyright (c) 2007-2011 Texas Instruments Incorporated.  All rights reserved.
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
// This is part of revision 6852 of the RDK-BLDC Firmware Package.
//
//*****************************************************************************

#include "ui_ethernet.h"
#include "ui_common.h"
#include "ui_commands.h"

//*****************************************************************************
//
//! \page ui_ethernet_intro Ethernet User Interface - Introduction
//!
//! A generic, TCP packet-based protocol is utilized for communicating with
//! the motor drive board.  This provides a method to control the motor drive,
//! adjust its parameters, and retrieve real-time performance data.  .
//!
//! - The same protocol should be used for all motor drive boards,
//!   regardless of the motor type (that is, AC induction, stepper, and
//!   so on).
//! - The protocol should make reasonable attempts to protect against invalid
//!   commands being acted upon.
//! - It should be possible to connect to a running motor drive board and lock
//!   on to the real-time data stream without having to restart the data
//!   stream. 
//!
//! The code for handling the ethernet protocol is contained in
//! <tt>ui_ethernet.c</tt>, with <tt>ui_ethernet.h</tt> containing the
//! definitions for the structures, functions, and variables exported to the
//! remainder of the application.  The file <tt>commands.h</tt> contains the
//! definitions for the commands, parameters, real-time data items, and
//! responses that are used in the ethernet protocol.  The file
//! <tt>ui_common.h</tt> contains the definitions that are common to all of
//! the communications API modules that are supported by the motor RDKs.
//!
//! The ethernet module builds on the serial module in that the same message
//! format is used.  This message is then transmitted/received using a TCP/IP
//! connection between the RDK board and the host application.  The following
//! message formats are defined:
//!
//! \section cmf Command Message Format
//!
//! Commands are sent to the motor drive with the following format:
//!
//! \code
//!     {tag} {length} {command} {optional command data byte(s)} {checksum}
//! \endcode
//!
//! - The {tag} byte is 0xff.
//!
//! - The {length} byte contains the overall length of the command packet,
//!   starting with the {tag} and ending with the {checksum}.  The maximum
//!   packet length is 255 bytes.
//!
//! - The {command} byte is the command being sent.  Based on the command,
//!   there may be optional command data bytes that follow.
//!
//! - The {checksum} byte is the value such that the sum of all bytes in the
//!   command packet (including the checksum) will be zero.  This is used to
//!   validate a command packet and allow the target to synchronize with the
//!   command stream being sent by the host.
//!
//! For example, the 0x01 command with no data bytes would be sent as
//! follows:
//!
//! \code
//!     0xff 0x04 0x01 0xfc
//! \endcode
//!
//! And the 0x02 command with two data bytes (0xab and 0xcd) would be sent as
//! follows:
//!
//! \code
//!     0xff 0x06 0x02 0xab 0xcd 0x81
//! \endcode
//!
//! \section smf Status Message Format
//!
//! Status messages are sent from the motor drive with the following format:
//!
//! \code
//!     {tag} {length} {data bytes} {checksum}
//! \endcode
//!
//! - The {tag} byte is 0xfe for command responses and 0xfd for real-time data.
//!
//! - The {length} byte contains the overall length of the status packet,
//!   starting with the {tag} byte and ending with the {checksum}.
//!
//! - The contents of the data bytes are dependent upon the tag byte.
//!
//! - The {checksum} is the value such that the sum of all bytes in the status
//!   packet (including the checksum) will be zero.  This is used to validate a
//!   status packet and allow the user interface to synchronize with the status
//!   stream being sent by the target.
//!
//! For command responses ({tag} = 0xfe), the first data byte is the command
//! that is being responded to.  The remaining bytes are the response, and are
//! dependent upon the command.
//!
//! For real-time data messages ({tag} = 0xfd), each real-time data item is
//! transmitted as a little-endian value (for example, for a 16-bit value, the
//! lower 8 bits first then the upper 8 bits).  The data items are in the same
//! order as returned by the data item list (#CMD_GET_DATA_ITEMS) regardless of
//! the order that they were enabled.
//!
//! For example, if data items 1, 5, and 17 were enabled, and each was two
//! bytes in length, there would be 6 data bytes in the packet:
//!
//! \code
//!     0xfd 0x09 {d1[0:7]} {d1[8:15]} {d5[0:7]} {d5[8:15]} {d17[0:7]}
//!          {d17[8:15]} {checksum}
//! \endcode
//!
//! \section pi Parameter Interpretation
//!
//! The size and units of the parameters are dependent upon the motor drive;
//! the units are not conveyed in the serial protocol.  Each parameter value is
//! transmitted in little endian format.  Not all parameters are necessarily
//! supported by a motor drive, only those that are appropriate.
//!
//! \section itta Interface To The Application
//!
//! The ethernet protocol handler takes care of all the ethernet communications
//! and command interpretation.  A set of functions provided by the application
//! and an array of structures that describe the parameters and real-time data
//! items supported by the motor drive.  The functions are used when an
//! application-specific action needs to take place as a result of the ethernet
//! communication (such as starting the motor drive).  The structures are used
//! to handle the parameters and real-time data items of the motor drive.
//
//*****************************************************************************

//*****************************************************************************
//
//! \defgroup ui_ethernet_api Ethernet User Interface - Definitions
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! A buffer to contain data received from the Ethernet.  A packet is processed 
//! out of this buffer once the entire packet is contained within the buffer.
//
//*****************************************************************************
static unsigned char g_pucUIEthernetReceive[ UIETHERNET_MAX_RECV ];

//*****************************************************************************
//
//! The offset of the next byte to be read from g_pucUIEthernetReceive.
//
//*****************************************************************************
static unsigned long g_ulUIEthernetReceiveRead;

//*****************************************************************************
//
//! The offset of the next byte to be written to g_pucUIEthernetReceive.
//
//*****************************************************************************
static unsigned long g_ulUIEthernetReceiveWrite;

//*****************************************************************************
//
//! A buffer used to construct status packets before they are written to the
//! UART and/or g_pucUIEthernetTransmit.
//
//*****************************************************************************
static unsigned char g_pucUIEthernetResponse[UIETHERNET_MAX_XMIT];

//*****************************************************************************
//
//! A buffer used to construct real-time data packets before they are written
//! to the UART and/or g_pucUIEthernetTransmit.
//
//*****************************************************************************
static unsigned char g_pucUIEthernetData[UIETHERNET_MAX_XMIT];

//*****************************************************************************
//
//! A boolean that is true when the real-time data stream is enabled.
//
//*****************************************************************************
static tBoolean g_bEnableRealTimeData;

//*****************************************************************************
//
//! Pointer to the telnet session PCB data structure.
//
//*****************************************************************************
static struct tcp_pcb *g_psTelnetPCB = NULL;

//*****************************************************************************
//
//! Running count updated by the UI System Tick Handler for milliseconds.
//
//*****************************************************************************
volatile unsigned long g_ulEthernetTimer = 0;

//*****************************************************************************
//
//! Flag to indicate that a Real Time Data update is ready for transmission.
//
//*****************************************************************************
static tBoolean g_bSendRealTimeData = false;

//*****************************************************************************
//
//! This global is used to store the number of Ethernet messages that have been
//! received since power-up.
//
//*****************************************************************************
volatile unsigned long g_ulEthernetRXCount = 0;

//*****************************************************************************
//
//! This global is used to store the number of Ethernet messages that have been
//! transmitted since power-up.
//
//*****************************************************************************
volatile unsigned long g_ulEthernetTXCount = 0;

//*****************************************************************************
//
//! Counter for TCP connection timeout.
//
//*****************************************************************************
static unsigned long g_ulConnectionTimeout = 0;

//*****************************************************************************
//
//! Timeout value for TCP connection timeout timer.
//
//*****************************************************************************
volatile unsigned long g_ulConnectionTimeoutParameter = UI_CONNECTION_TIMEOUT;

//*****************************************************************************
//
//! Close an existing Ethernet connection.
//!
//! \param pcb is the pointer to the TCP control structure.
//! 
//! This function is called when the the TCP connection should be closed.
//!
//! \return None.
//
//*****************************************************************************
static void
UIEthernetClose(struct tcp_pcb *pcb)
{
    //
    // Clear out all of the TCP callbacks.
    //
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 1);

    //
    // Clear the telnet data structure pointer, to indicate that
    // we are no longer connected.
    //
    g_psTelnetPCB = NULL;

    //
    // Close the TCP connection.
    //
    tcp_close(pcb);
}

//*****************************************************************************
//
//! Transmits a packet to the Ethernet controller.
//!
//! \param pucBuffer is a pointer to the packet to be transmitted.
//!
//! This function will send a packet via TCP/Ethernet.  It will compute the
//! checksum of the packet (based on the length in the second byte) and place
//! it at the end of the packet before sending the packet.
//!
//! \return Returns \b true if the entire packet was transmitted and \b false
//! if not.
//
//*****************************************************************************
unsigned long g_ulTxOutError = 0;
unsigned long g_ulTxWriteError = 0;
static tBoolean
UIEthernetTransmit(unsigned char *pucBuffer)
{
    static unsigned long ulIdx, ulSum, ulLength;
    static err_t err;

    //
    // Compute the checksum for this packet and put it at the end.
    //
    for(ulIdx = 0, ulSum = 0, ulLength = pucBuffer[1]; ulIdx < (ulLength - 1);
        ulIdx++)
    {
        ulSum -= pucBuffer[ulIdx];
    }
    pucBuffer[ulLength - 1] = ulSum;

    //
    // Transmit the packet and flush the buffer to get the packet out
    // immediately.
    //
    err = tcp_write(g_psTelnetPCB, pucBuffer, ulLength, 1);
    if(err == ERR_OK)
    {
        g_ulEthernetTXCount++;
        err = tcp_output(g_psTelnetPCB);
        if(err != ERR_OK)
        {
            g_ulTxOutError++;
        }
    }
    else
    {
        g_ulTxWriteError++;
    }
    
    //
    // Return an error if there are characters to be sent that would not fit
    // into the transmit buffer.
    //
    if(err != ERR_OK)
    {
        return false;
    }

    return true;
}

//*****************************************************************************
//
//! Finds a parameter by ID.
//!
//! \param ucID is the ID of the parameter to locate.
//!
//! This function searches the list of parameters looking for one that matches
//! the provided ID.
//!
//! \return Returns the index of the parameter found, or 0xffff.ffff if the
//! parameter does not exist in the parameter list.
//
//*****************************************************************************
static unsigned long
UIEthernetFindParameter(unsigned char ucID)
{
    static unsigned long ulIdx;

    //
    // Loop through the parameters.
    //
    for(ulIdx = 0; ulIdx < g_ulUINumParameters; ulIdx++)
    {
        //
        // See if this parameter matches the search ID.
        //
        if(g_sUIParameters[ulIdx].ucID == ucID)
        {
            //
            // Since this parameter matches, return its index.
            //
            return(ulIdx);
        }
    }

    //
    // The parameter could not be located, so return a failure.
    //
    return(0xffffffff);
}

//*****************************************************************************
//
//! Performs range checking on the value of a parameter.
//!
//! \param ulIdx is the index of the parameter to check.
//!
//! This function will perform range checking on the value of a parameter,
//! adjusting the parameter value if necessary to make it reside within the
//! predetermined range.
//!
//! \return None.
//
//*****************************************************************************
static void
UIEthernetRangeCheck(unsigned long ulIdx)
{
    static unsigned long *pulValue;
    static unsigned short *pusValue;
    static long *plValue, lCheck;
    static short *psValue, sCheck;
    static char *pcValue, cCheck;

    //
    // See if a range exists for this parameter.  If not, then return without
    // checking the parameter's value.
    //
    if(((g_sUIParameters[ulIdx].ulMin == 0) &&
        (g_sUIParameters[ulIdx].ulMax == 0)) ||
       (g_sUIParameters[ulIdx].ucSize > 4))
    {
        return;
    }

    //
    // See if the value of this parameter is signed or unsigned.
    //
    if(g_sUIParameters[ulIdx].ulMin > g_sUIParameters[ulIdx].ulMax)
    {
        //
        // This is a signed parameter.  Determine the size of the parameter.
        //
        switch(g_sUIParameters[ulIdx].ucSize)
        {
            //
            // This parameter is a signed character.
            //
            case 1:
            {
                //
                // Get the signed value of this parameter.
                //
                pcValue = (char *)(g_sUIParameters[ulIdx].pucValue);

                //
                // Clamp the parameter value to the minimum if it is below the
                // minimum.
                //
                cCheck = (char)g_sUIParameters[ulIdx].ulMin;
                if(*pcValue < cCheck)
                {
                    *pcValue = cCheck;
                }

                //
                // Clamp the parameter value to the maximum if it is above the
                // maximum.
                //
                cCheck = (char)g_sUIParameters[ulIdx].ulMax;
                if(*pcValue > cCheck)
                {
                    *pcValue = cCheck;
                }

                //
                // This parameter value has been handled.
                //
                break;
            }

            //
            // This parameter is a signed short.
            //
            case 2:
            {
                //
                // Get the signed value of this parameter.
                //
                psValue = (short *)(g_sUIParameters[ulIdx].pucValue);

                //
                // Clamp the parameter value to the minimum if it is below the
                // minimum.
                //
                sCheck = (short)g_sUIParameters[ulIdx].ulMin;
                if(*psValue < sCheck)
                {
                    *psValue = sCheck;
                }

                //
                // Clamp the parameter value to the maximum if it is above the
                // maximum.
                //
                sCheck = (short)g_sUIParameters[ulIdx].ulMax;
                if(*psValue > sCheck)
                {
                    *psValue = sCheck;
                }

                //
                // This parameter value has been handled.
                //
                break;
            }

            //
            // This parameter is a signed long.
            //
            case 4:
            default:
            {
                //
                // Get the signed value of this parameter.
                //
                plValue = (long *)(g_sUIParameters[ulIdx].pucValue);

                //
                // Clamp the parameter value to the minimum if it is below the
                // minimum.
                //
                lCheck = (long)g_sUIParameters[ulIdx].ulMin;
                if(*plValue < lCheck)
                {
                    *plValue = lCheck;
                }

                //
                // Clamp the parameter value to the maximum if it is above the
                // maximum.
                //
                lCheck = (long)g_sUIParameters[ulIdx].ulMax;
                if(*plValue > lCheck)
                {
                    *plValue = lCheck;
                }

                //
                // This parameter value has been handled.
                //
                break;
            }
        }
    }
    else
    {
        //
        // This is an unsigned parameter.  Determine the size of the parameter.
        //
        switch(g_sUIParameters[ulIdx].ucSize)
        {
            //
            // This parameter is an unsigned character.
            //
            case 1:
            {
                //
                // Clamp the parameter value to the minimum if it is below the
                // minimum.
                //
                if(g_sUIParameters[ulIdx].pucValue[0] <
                   g_sUIParameters[ulIdx].ulMin)
                {
                    g_sUIParameters[ulIdx].pucValue[0] =
                        g_sUIParameters[ulIdx].ulMin;
                }

                //
                // Clamp the parameter value to the maximum if it is above the
                // maximum.
                //
                if(g_sUIParameters[ulIdx].pucValue[0] >
                   g_sUIParameters[ulIdx].ulMax)
                {
                    g_sUIParameters[ulIdx].pucValue[0] =
                        g_sUIParameters[ulIdx].ulMax;
                }

                //
                // This parameter value has been handled.
                //
                break;
            }

            //
            // This parameter is an unsigned short.
            //
            case 2:
            {
                //
                // Get the value of this parameter.
                //
                pusValue = (unsigned short *)(g_sUIParameters[ulIdx].pucValue);

                //
                // Clamp the parameter value to the minimum if it is below the
                // minimum.
                //
                if(*pusValue < g_sUIParameters[ulIdx].ulMin)
                {
                    *pusValue = g_sUIParameters[ulIdx].ulMin;
                }

                //
                // Clamp the parameter value to the maximum if it is above the
                // maximum.
                //
                if(*pusValue > g_sUIParameters[ulIdx].ulMax)
                {
                    *pusValue = g_sUIParameters[ulIdx].ulMax;
                }

                //
                // This parameter value has been handled.
                //
                break;
            }

            //
            // This parameter is an unsigned long.
            //
            case 4:
            default:
            {
                //
                // Get the value of this parameter.
                //
                pulValue = (unsigned long *)(g_sUIParameters[ulIdx].pucValue);

                //
                // Clamp the parameter value to the minimum if it is below the
                // minimum.
                //
                if(*pulValue < g_sUIParameters[ulIdx].ulMin)
                {
                    *pulValue = g_sUIParameters[ulIdx].ulMin;
                }

                //
                // Clamp the parameter value to the maximum if it is above the
                // maximum.
                //
                if(*pulValue > g_sUIParameters[ulIdx].ulMax)
                {
                    *pulValue = g_sUIParameters[ulIdx].ulMax;
                }

                //
                // This parameter value has been handled.
                //
                break;
            }
        }
    }
}

//*****************************************************************************
//
//! Scans for packets in the receive buffer.
//!
//! This function will scan through g_pucUIEthernetReceive looking for valid
//! command packets.  When found, the command packets will be handled.
//!
//! \return None.
//
//*****************************************************************************

static void
UIEthernetScanReceive(void)
{
    unsigned char ucSum, ucSize;
    unsigned long ulIdx;
    IMUSensorData* sIMUSensorData;

    UINT16 m1_ref, m2_ref, m3_ref;

    //
    // Loop while there is data in the receive buffer.
    //
    while(g_ulUIEthernetReceiveRead != g_ulUIEthernetReceiveWrite)
    {
        //
        // See if this character is the tag for the start of a command packet.
        //
        if(g_pucUIEthernetReceive[g_ulUIEthernetReceiveRead] != TAG_CMD)
        {
            //
            // Skip this character.
            //
            g_ulUIEthernetReceiveRead =
                (g_ulUIEthernetReceiveRead + 1) % UIETHERNET_MAX_RECV;

            //
            // Keep scanning for a start of command packet tag.
            //
            continue;
        }

        //
        // See if there are additional characters in the receive buffer.
        //
        if(((g_ulUIEthernetReceiveRead + 1) % UIETHERNET_MAX_RECV) ==
           g_ulUIEthernetReceiveWrite)
        {
            //
            // There are no additional characters in the receive buffer after
            // the start of command packet byte, so stop scanning for now.
            //
            break;
        }

        //
        // See if the packet size byte is valid.  A command packet must be at
        // least four bytes and can not be larger than the receive buffer size.
        //
        ucSize = g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 1) %
                                      UIETHERNET_MAX_RECV];
        if((ucSize < 4) || (ucSize > (UIETHERNET_MAX_RECV - 1)))
        {
            //
            // The packet size is too large, so either this is not the start of
            // a packet or an invalid packet was received.  Skip this start of
            // command packet tag.
            //
            g_ulUIEthernetReceiveRead =
                (g_ulUIEthernetReceiveRead + 1) % UIETHERNET_MAX_RECV;

            //
            // Keep scanning for a start of command packet tag.
            //
            continue;
        }

        //
        // Determine the number of bytes in the receive buffer.
        //
        ulIdx = g_ulUIEthernetReceiveWrite - g_ulUIEthernetReceiveRead;
        if(g_ulUIEthernetReceiveRead > g_ulUIEthernetReceiveWrite)
        {
            ulIdx += UIETHERNET_MAX_RECV;
        }

        //
        // If the entire command packet is not in the receive buffer then stop
        // scanning for now.
        //
        if(ulIdx < ucSize)
        {
            break;
        }

        //
        // The entire command packet is in the receive buffer, so compute its
        // checksum.
        //
        for(ulIdx = 0, ucSum = 0; ulIdx < ucSize; ulIdx++)
        {
            ucSum += g_pucUIEthernetReceive[
                (g_ulUIEthernetReceiveRead + ulIdx) % UIETHERNET_MAX_RECV];
        }

        //
        // Skip this packet if the checksum is not correct (that is, it is
        // probably not really the start of a packet).
        //
        if(ucSum != 0)
        {
            //
            // Skip this character.
            //
            g_ulUIEthernetReceiveRead =
                (g_ulUIEthernetReceiveRead + 1) % UIETHERNET_MAX_RECV;

            //
            // Keep scanning for a start of command packet tag.
            //
            continue;
        }

        //
        // A valid command packet was received, so process it now.
        //
        switch(g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 2) %
                                    UIETHERNET_MAX_RECV])
        {
            //
            // The command to get the target type.
            //
            case CMD_ID_TARGET:
            {
                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x05;
                g_pucUIEthernetResponse[2] = CMD_ID_TARGET;
                g_pucUIEthernetResponse[3] = g_ulUITargetType;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to upgrade the firmware.
            //
            case CMD_UPGRADE:
            {
                //
                // Clear out all of the TCP callbacks.
                //
                tcp_arg(g_psTelnetPCB, NULL);
                tcp_sent(g_psTelnetPCB, NULL);
                tcp_recv(g_psTelnetPCB, NULL);
                tcp_err(g_psTelnetPCB, NULL);
                tcp_poll(g_psTelnetPCB, NULL, 1);

                //
                // Close the TCP connection.
                //
                tcp_abort(g_psTelnetPCB);

                //
                // Clear the telnet data structure pointer, to indicate that
                // we are no longer connected.
                //
                g_psTelnetPCB = NULL;

                //
                // Pass the upgrade request to the application.
                //
                UIUpgrade();

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to get a list of the parameters.
            //
            case CMD_GET_PARAMS:
            {
                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = g_ulUINumParameters + 4;
                g_pucUIEthernetResponse[2] = CMD_GET_PARAMS;
                for(ulIdx = 0; ulIdx < g_ulUINumParameters; ulIdx++)
                {
                    g_pucUIEthernetResponse[ulIdx + 3] =
                        g_sUIParameters[ulIdx].ucID;
                }

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to get a description of a parameter.
            //
            case CMD_GET_PARAM_DESC:
            {
                //
                // Find the parameter.
                //
                ucSum =
                    g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 3) %
                                         UIETHERNET_MAX_RECV];
                ulIdx = UIEthernetFindParameter(ucSum);

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[2] = CMD_GET_PARAM_DESC;

                //
                // If a parameter was not specified, or if the parameter could
                // not be found, then return a zero length.
                //
                if((ucSize != 5) || (ulIdx == 0xffffffff))
                {
                    g_pucUIEthernetResponse[1] = 0x05;
                    g_pucUIEthernetResponse[3] = 0x00;
                }

                //
                // If the length of the parameter is greater than a 32-bit
                // value, then return just the size of the parameter.
                //
                else if(g_sUIParameters[ulIdx].ucSize > 4)
                {
                    g_pucUIEthernetResponse[1] = 0x05;
                    g_pucUIEthernetResponse[3] = g_sUIParameters[ulIdx].ucSize;
                }

                //
                // Otherwise, return the size, minimum, maximum, and step size
                // for the parameter.
                //
                else
                {
                    //
                    // Set the size of the response packet.
                    //
                    g_pucUIEthernetResponse[1] =
                        (g_sUIParameters[ulIdx].ucSize * 3) + 5;

                    //
                    // Set the size of the parameter value.
                    //
                    g_pucUIEthernetResponse[3] = g_sUIParameters[ulIdx].ucSize;

                    //
                    // Loop through the bytes of the parameter value.
                    //
                    ucSize = g_sUIParameters[ulIdx].ucSize;
                    for(ucSum = 0; ucSum < ucSize; ucSum++)
                    {
                        //
                        // Set this byte of the parameter minimum value.
                        //
                        g_pucUIEthernetResponse[ucSum + 4] =
                            ((g_sUIParameters[ulIdx].ulMin >> (ucSum * 8)) &
                             0xff);

                        //
                        // Set this byte of the parameter maximum value.
                        //
                        g_pucUIEthernetResponse[ucSum + ucSize + 4] =
                            ((g_sUIParameters[ulIdx].ulMax >> (ucSum * 8)) &
                             0xff);

                        //
                        // Set this byte of the parameter step size.
                        //
                        g_pucUIEthernetResponse[ucSum + (ucSize << 1) + 4] =
                            ((g_sUIParameters[ulIdx].ulStep >> (ucSum * 8)) &
                             0xff);
                    }
                }

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to get the value of a parameter.
            //
            case CMD_GET_PARAM_VALUE:
            {
                //
                // Find the parameter.
                //
                ucSum =
                    g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 3) %
                                         UIETHERNET_MAX_RECV];
                ulIdx = UIEthernetFindParameter(ucSum);

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[2] = CMD_GET_PARAM_VALUE;

                //
                // If a parameter was not specified, or if the parameter could
                // not be found, then return no value.
                //
                if((ucSize != 5) || (ulIdx == 0xffffffff))
                {
                    g_pucUIEthernetResponse[1] = 0x04;
                }

                //
                // Return the current value of the parameter.
                //
                else
                {
                    //
                    // Set the response packet size based on the size of the
                    // parameter.
                    //
                    g_pucUIEthernetResponse[1] =
                        g_sUIParameters[ulIdx].ucSize + 4;

                    //
                    // Copy the parameter value to the response packet.
                    //
                    for(ucSum = 0; ucSum < g_sUIParameters[ulIdx].ucSize;
                        ucSum++)
                    {
                        g_pucUIEthernetResponse[ucSum + 3] =
                            g_sUIParameters[ulIdx].pucValue[ucSum];
                    }
                }

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to set the value of a parameter.
            //
            case CMD_SET_PARAM_VALUE:
            {
                //
                // Find the parameter.
                //
                ucSum =
                    g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 3) %
                                         UIETHERNET_MAX_RECV];
                ulIdx = UIEthernetFindParameter(ucSum);

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_SET_PARAM_VALUE;

                //
                // Only set the value of the parameter if a value was
                // specified, the parameter could be found, and the parameter
                // is not read-only.
                //
                if((ucSize > 5) && (ulIdx != 0xffffffff) &&
                   (g_sUIParameters[ulIdx].ulStep != 0))
                {
                    //
                    // Loop through the bytes of this parameter value.
                    //
                    for(ucSum = 0; ucSum < g_sUIParameters[ulIdx].ucSize;
                        ucSum++)
                    {
                        //
                        // See if this byte was supplied.
                        //
                        if(ucSum < (ucSize - 5))
                        {
                            //
                            // Set this byte of the parameter value based on
                            // the supplied byte.
                            //
                            g_sUIParameters[ulIdx].pucValue[ucSum] =
                                g_pucUIEthernetReceive[
                                (g_ulUIEthernetReceiveRead + ucSum + 4) %
                                UIETHERNET_MAX_RECV];
                        }
                        else
                        {
                            //
                            // Set this byte of the parameter value to zero
                            // since it was not supplied.
                            //
                            g_sUIParameters[ulIdx].pucValue[ucSum] = 0;
                        }
                    }

                    //
                    // Perform range checking on the parameter value.
                    //
                    UIEthernetRangeCheck(ulIdx);

                    //
                    // If there is an update function for this parameter then
                    // call it now.
                    //
                    if(g_sUIParameters[ulIdx].pfnUpdate)
                    {
                        g_sUIParameters[ulIdx].pfnUpdate();
                    }
                }

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to load parameters from flash.
            //
            case CMD_LOAD_PARAMS:
            {
                //
                // Pass the parameter load request to the application.
                //
                UIParamLoad();

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_LOAD_PARAMS;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to save parameters to flash.
            //
            case CMD_SAVE_PARAMS:
            {
                //
                // Pass the parameter save request to the application.
                //
                UIParamSave();

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_LOAD_PARAMS;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to get a list of the real-time data items.
            //
            case CMD_GET_DATA_ITEMS:
            {
                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = (g_ulUINumRealTimeData * 2) + 4;
                g_pucUIEthernetResponse[2] = CMD_GET_DATA_ITEMS;
                for(ulIdx = 0; ulIdx < g_ulUINumRealTimeData; ulIdx++)
                {
                    g_pucUIEthernetResponse[(ulIdx * 2) + 3] =
                        g_sUIRealTimeData[ulIdx].ucID;
                    g_pucUIEthernetResponse[(ulIdx * 2) + 4] =
                        g_sUIRealTimeData[ulIdx].ucSize;
                }

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to start the real-time data stream.
            //
            case CMD_START_DATA_STREAM:
            {
                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_START_DATA_STREAM;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Enable the real-time data stream.
                //
                g_bEnableRealTimeData = true;

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to stop the real-time data stream.
            //
            case CMD_STOP_DATA_STREAM:
            {
                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_STOP_DATA_STREAM;

                //
                // Disable the real-time data stream.
                //
                g_bEnableRealTimeData = false;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to start the motor drive.
            //
            case CMD_RUN:
            {
                //
                // Pass the run request to the application.
                //
                UIRun();

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_RUN;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command to stop the motor drive.
            //
            case CMD_STOP:
            {
                //
                // Pass the stop request to the application.
                //
                UIStop();

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_STOP;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            //
            // The command for an emmergency stop of the motor drive.
            //
            case CMD_EMERGENCY_STOP:
            {
                //
                // Pass the emergency stop request to the application.
                //
                UIEmergencyStop();

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_EMERGENCY_STOP;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            case CMD_SET_MOTOR_REFS:
            {
            	//
            	// Read position references for the motors from the input stream and
            	// apply them.
            	//

            	m1_ref = ((UINT16)g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 3) % UIETHERNET_MAX_RECV] << 8 ) +
            			g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 4) % UIETHERNET_MAX_RECV];
            	m2_ref = ((UINT16)g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 5) % UIETHERNET_MAX_RECV] << 8 ) +
            			g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 6) % UIETHERNET_MAX_RECV];
            	m3_ref = ((UINT16)g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 7) % UIETHERNET_MAX_RECV] << 8 ) +
            			g_pucUIEthernetReceive[(g_ulUIEthernetReceiveRead + 8) % UIETHERNET_MAX_RECV];

            	RCServoUpdate( (INT16)m1_ref, (INT16)m2_ref, (INT16)m3_ref );

                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x04;
                g_pucUIEthernetResponse[2] = CMD_SET_MOTOR_REFS;

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
                break;
            }

            case CMD_GET_SENSOR_DATA:
            {
                //
                // Fill in the response.
                //
                g_pucUIEthernetResponse[0] = TAG_STATUS;
                g_pucUIEthernetResponse[1] = 0x16;
                g_pucUIEthernetResponse[2] = CMD_GET_SENSOR_DATA;

                sIMUSensorData = IMUGetSensorData();

                //
                // Here, variable m1_ref serves as a temporary variable
                //

                m1_ref = sIMUSensorData->XGyro;
                g_pucUIEthernetResponse[3] = m1_ref >> 8;
                g_pucUIEthernetResponse[4] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->YGyro;
                g_pucUIEthernetResponse[5] = m1_ref >> 8;
                g_pucUIEthernetResponse[6] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->ZGyro;
                g_pucUIEthernetResponse[7] = m1_ref >> 8;
                g_pucUIEthernetResponse[8] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->XAccl;
                g_pucUIEthernetResponse[9] = m1_ref >> 8;
                g_pucUIEthernetResponse[10] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->YAccl;
                g_pucUIEthernetResponse[11] = m1_ref >> 8;
                g_pucUIEthernetResponse[12] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->ZAccl;
                g_pucUIEthernetResponse[13] = m1_ref >> 8;
                g_pucUIEthernetResponse[14] = m1_ref & 0xFF;

#ifdef USE_ADIS16405
                m1_ref = sIMUSensorData->XMagn;
                g_pucUIEthernetResponse[15] = m1_ref >> 8;
                g_pucUIEthernetResponse[16] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->YMagn;
                g_pucUIEthernetResponse[17] = m1_ref >> 8;
                g_pucUIEthernetResponse[18] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->ZMagn;
                g_pucUIEthernetResponse[19] = m1_ref >> 8;
                g_pucUIEthernetResponse[20] = m1_ref & 0xFF;
#endif

#ifdef USE_ADIS16367
                m1_ref = sIMUSensorData->XTemp;
                g_pucUIEthernetResponse[15] = m1_ref >> 8;
                g_pucUIEthernetResponse[16] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->YTemp;
                g_pucUIEthernetResponse[17] = m1_ref >> 8;
                g_pucUIEthernetResponse[18] = m1_ref & 0xFF;

                m1_ref = sIMUSensorData->ZTemp;
                g_pucUIEthernetResponse[19] = m1_ref >> 8;
                g_pucUIEthernetResponse[20] = m1_ref & 0xFF;
#endif

                //
                // Send the response.
                //
                UIEthernetTransmit(g_pucUIEthernetResponse);

                //
                // Done with this command.
                //
            }

            //
            // An unrecognized command was received.  Simply ignore it.
            //
            default:
            {
                //
                // Done with this command.
                //
                break;
            }
        }

        //
        // Skip this command packet.
        //
        g_ulUIEthernetReceiveRead =
            (g_ulUIEthernetReceiveRead + ucSize) % UIETHERNET_MAX_RECV;
    }
}

//*****************************************************************************
//
//! Callback for Ethernet transmit.
//!
//! \param arg is not used in this implementation.
//! \param pcb is not used in this implementation.
//! \param len is not used in this implementation.
//!
//! This function is called when the lwIP TCP/IP stack has received an
//! acknowledgement for data that has been transmitted.
//!
//! \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
UIEthernetSent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    //
    // Reset the connection timeout.
    //
    g_ulConnectionTimeout = 0;

    //
    // Return OK.
    //
    return(ERR_OK);
}

//*****************************************************************************
//
//! Receive a TCP packet from lwIP for motor control processing.
//!
//! \param arg is not used in this implementation.
//! \param pcb is the pointer to the TCP control structure.
//! \param p is the pointer to the PBUF structure containing the packet data.
//! \param err is used to indicate if any errors are associated with the
//! incoming packet.
//!
//! This function is called when the lwIP TCP/IP stack has an incoming
//! packet to be processed.
//!
//! \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
UIEthernetReceive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct pbuf *q;
    unsigned long ulIdx;
    unsigned char *pucData;

    //
    // Process the incoming packet.
    // 
    if((err == ERR_OK) && (p != NULL))
    {
        //
        // Increment the packet counter.
        //
        g_ulEthernetRXCount++;

        //
        // Accept the packet from TCP.
        // 
        tcp_recved(pcb, p->tot_len);

        //
        // Process the packet.
        //
        for(q = p, pucData = q->payload; q != NULL; q = q->next)
        {
            for(ulIdx = 0; ulIdx < q->len; ulIdx++)
            {
                //
                // Copy the next character into the receive buffer.
                //
                g_pucUIEthernetReceive[g_ulUIEthernetReceiveWrite] =
                    pucData[ulIdx];

                //
                // Increment the receive buffer write pointer.
                //
                g_ulUIEthernetReceiveWrite =
                    (g_ulUIEthernetReceiveWrite + 1) % UIETHERNET_MAX_RECV;

                //
                // Scan the receive buffer for command packets if it is full.
                //
                if(((g_ulUIEthernetReceiveWrite + 1) % UIETHERNET_MAX_RECV) ==
                    g_ulUIEthernetReceiveRead)
                {
                    UIEthernetScanReceive();
                }
            }
        }

        //
        // Scan the receive buffer for command packets.
        //
        UIEthernetScanReceive();

        //
        // Free the pbuf.
        //
        pbuf_free(p);
    }

    //
    // If a null packet is passed in, close the connection.
    //
    else if((err == ERR_OK) && (p == NULL))
    {
        UIEthernetClose(pcb);
    }

    //
    // Return okay.
    //
    return ERR_OK;
}

//*****************************************************************************
//
//! lwIP TCP/IP Polling/Timeout function.
//!
//! \param arg is not used in this implementation.
//! \param pcb is not used in this implementation.
//!
//! This function is called when the lwIP TCP/IP stack has no incoming or
//! outgoing data.  It can be used to reset an idle connection.
//!
//! \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
UIEthernetPoll(void *arg, struct tcp_pcb *pcb)
{
    //
    // Check the connection timeout.
    //
    g_ulConnectionTimeout++;
    if(g_ulConnectionTimeoutParameter &&
       (g_ulConnectionTimeout > g_ulConnectionTimeoutParameter))
    {
        tcp_abort(g_psTelnetPCB);
        g_psTelnetPCB = NULL;
    }
    return ERR_OK;
}

//*****************************************************************************
//
//! lwIP TCP/IP error handling.
//!
//! \param arg is not used in this implementation.
//! \param err is not used in this implementation.
//!
//! This function is called when the lwIP TCP/IP stack has detected an
//! error.  The connection is no longer valid.
//!
//! \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static void
UIEthernetError(void *arg, err_t err)
{
    //
    // Reset our connection.
    //
    g_psTelnetPCB = NULL;
}

//*****************************************************************************
//
//! Accept a TCP connection for motor control processing.
//!
//! \param arg is not used in this implementation.
//! \param pcb is the pointer to the TCP control structure.
//! \param err is not used in this implementation.
//!
//! This function is called when the lwIP TCP/IP stack has an incoming
//! connection request on the telnet port.
//!
//! \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t
UIEthernetAccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    //
    // Check if already connected.
    //
    if(g_psTelnetPCB)
    {
        //
        // If we already have a connection, kill it and start over.
        //
        UIEthernetClose(g_psTelnetPCB);
    }

    //
    // Set the connection timeout to 0.
    //
    g_ulConnectionTimeout = 0;

    //
    // Set the connection as busy.
    //
    g_psTelnetPCB = pcb;

    //
    // Disable the NAGLE algorithm.

    //
    g_psTelnetPCB->flags |= TF_NODELAY;

    //
    // Setup the TCP connection priority.
    //
    tcp_setprio(pcb, TCP_PRIO_MIN);

    //
    // Setup the TCP callback argument.
    //
    tcp_arg(pcb, NULL);

    //
    // Setup the TCP receive function.
    // 
    tcp_recv(pcb, UIEthernetReceive);

    //
    // Setup the TCP error function.
    //
    tcp_err(pcb, UIEthernetError);

    //
    // Setup the TCP polling function/interval.
    //
    tcp_poll(pcb, UIEthernetPoll, (1000/TCP_SLOW_INTERVAL));

    //
    // Setup the TCP sent callback function.
    //
    tcp_sent(pcb, UIEthernetSent);

    //
    // Return a success code.
    // 
    return ERR_OK;
}

//*****************************************************************************
//
//! Sends a real-time data packet.
//!
//! This function will construct a real-time data packet with the current
//! values of the enabled real-time data items.  Once constructed, the packet
//! will be sent out.
//!
//! \return None.
//
//*****************************************************************************
void
UIEthernetSendRealTimeData(void)
{
    static unsigned long ulPos, ulItem, ulCount;
    static unsigned char pucValue[4];
    static tBoolean bReady = true;

    //
    // Do nothing if the real-time data stream is not enabled.
    //
    if(!g_bEnableRealTimeData)
    {
        return;
    }

    //
    // Do nothing if the last real-time data stream has not been sent.
    //
    if(g_bSendRealTimeData)
    {
        return;
    }

    //
    // Protect re-entrancy here.
    //
    if(!bReady)
    {
        return;
    }
    bReady = false;

    //
    // Loop through the real-time data items.
    //
    for(ulItem = 0, ulPos = 2; ulItem < g_ulUINumRealTimeData; ulItem++)
    {
		//
		// Perform an atomic copy of the value to a local variable.  This
		// prevents the value from being changed midway through adding it
		// to the output real-time data stream.
		//
		*((unsigned long *)pucValue) =
		*((unsigned long *)g_sUIRealTimeData[ulItem].pucValue);
		
		//
		// Copy the value of this real-time data item, byte by byte, to the
		// packet.
		//
		for(ulCount = 0; ulCount < g_sUIRealTimeData[ulItem].ucSize;
			ulCount++)
		{
			g_pucUIEthernetData[ulPos++] = pucValue[ulCount];
		}
    }

    //
    // Put the header and length on the real-time data packet.
    //
    g_pucUIEthernetData[0] = TAG_DATA;
    g_pucUIEthernetData[1] = ulPos + 1;

    //
    // Indicate that real time data is being prepared to send.
    //
    g_bSendRealTimeData = true;
    bReady = true;
}

//*****************************************************************************
//
//! Return the current IPV4 TCP/IP address.
//!
//! Read the IP address from the lwIP network interface structure, and return
//! it in unsigned long format.
//!
//! \return IP address.
//
//*****************************************************************************
unsigned long
UIEthernetGetIPAddress(void)
{
    //
    // Return the current IP address.
    //
    return(lwIPLocalIPAddrGet());
}

//*****************************************************************************
//
//! Initialize the Ethernet controller and lwIP TCP/IP stack.
//!
//! \param bUseDHCP indicates whether the DHCP client software should
//! be used (if the build options have enabled it).
//!
//! Initialize the Ethernet controller for operation, including the
//! setup of the MAC address and enabling of status LEDs.  Also initialize
//! the lwIP TCP/IP stack for operation, including DHCP operation.
//!
//! \return None.
//
//*****************************************************************************
void
UIEthernetInit(tBoolean bUseDHCP)
{
    unsigned long ulUser0, ulUser1;
    unsigned char pucMACArray[8];
    void *pcb;

	//
	// Enable and Reset the Ethernet Controller.
	//
	ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_ETH );
	ROM_SysCtlPeripheralReset( SYSCTL_PERIPH_ETH );

	//
	// Enable Port F for Ethernet LEDs.
	//
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinConfigure(GPIO_PF2_LED1);
	GPIOPinConfigure(GPIO_PF3_LED0);
	GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);

	//
	// Read the MAC address from the user registers.
	//
	ROM_FlashUserGet(&ulUser0, &ulUser1);
	if((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff))
	{
		//
		// We should never get here.  This is an error if the MAC address has
		// not been programmed into the device.  Exit the program.
		//
		UARTprintf("MAC Address Not Programmed!\n");
		while( 1 )
		{
		}
	}

	//
	// Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
	// address needed to program the hardware registers, then program the MAC
	// address into the Ethernet Controller registers.
	//
	pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
	pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
	pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
	pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
	pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
	pucMACArray[5] = ((ulUser1 >> 16) & 0xff);

    //
    // Initialize lwIP
    //
    lwIPInit(
    	pucMACArray,
    	DEFAULT_IPADDR, DEFAULT_GATEWAY_ADDR, DEFAULT_NET_MASK,
    	(bUseDHCP ? IPADDR_USE_DHCP : IPADDR_USE_STATIC)
    );

    //
    // Initialize the application to listen on the telnet port.
    //
    pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, UI_PROTO_PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, UIEthernetAccept);

	//
	// Setup the device locator service.
	//
	LocatorInit( );
	LocatorMACAddrSet( pucMACArray );
	LocatorAppTitleSet("EK-LM3S9B92 Plane-Power control board");
}

//*****************************************************************************
//
//! Handles the Ethernet interrupt hooks for the client software.
//!
//! This function will run any handlers that are required to run inthe
//! Ethernet interrupt context.
//!
//! \return None.
//
//*****************************************************************************
void
lwIPHostTimerHandler(void)
{
    //
    // Send a real-time data packet, if one has been requested.
    //
    if(g_bSendRealTimeData)
    {
        UIEthernetTransmit( g_pucUIEthernetData );
        g_bSendRealTimeData = false;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
