//*****************************************************************************
//
// ui_ethernet.h - Prototypes for the simple TCP/IP control interface.
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

#ifndef __UI_ETHERNET_H__
#define __UI_ETHERNET_H__

#include "bsp.h"

#include "imu.h"
#include "rcservo.h"

//*****************************************************************************
//
//! The size of the transmit buffer.  This should be appropriately sized
//! such that the maximum burst of output data can be contained in this buffer.
//! This value should be a power of two in order to make the modulo arithmetic
//! be fast (that is, an AND instead of a divide).
//
//*****************************************************************************
#ifndef UIETHERNET_MAX_XMIT
#define UIETHERNET_MAX_XMIT     64
#endif

//*****************************************************************************
//
//! The size of the receive buffer.  This should be appropriately sized
//! such that the maximum size command packet can be contained in this buffer.
//! This value should be a power of two in order to make the modulo arithmetic
//! be fast (that is, an AND instead of a divide).
//
//*****************************************************************************
#ifndef UIETHERNET_MAX_RECV
#define UIETHERNET_MAX_RECV     64
#endif

//*****************************************************************************
//
//! Default TCP/IP Address Configuration.  Static IP Configuration is used if
//! DHCP times out.
//
//*****************************************************************************
//
//! The Default IP address to be used.
//
#ifndef DEFAULT_IPADDR
#define DEFAULT_IPADDR          ((169u << 24) | (254 << 16) | (89 << 8) | 71)
#endif

//
//! The Default Gateway address to be used.
//
#ifndef DEFAULT_GATEWAY_ADDR
#define DEFAULT_GATEWAY_ADDR    0
#endif

//
//! The Default Network mask to be used.
//
#ifndef DEFAULT_NET_MASK
#define DEFAULT_NET_MASK        ((255u << 24) | (255 << 16) | (0 << 8) | 0)
#endif

//*****************************************************************************
//
//! The port to use for TCP connections for the UI protocol.
//
//*****************************************************************************
#ifndef UI_PROTO_PORT
#define UI_PROTO_PORT           8003
#endif

//*****************************************************************************
//
//! Parameter definition for timeout value for TCP connection timeout timer.
//
//*****************************************************************************
#ifndef UI_CONNECTION_TIMEOUT
#define UI_CONNECTION_TIMEOUT 10
#endif

//*****************************************************************************
//
//! Parameter definition for timeout value for TCP connection timeout timer.
//
//*****************************************************************************
#ifndef UI_CONNECTION_TIMEOUT
#define UI_CONNECTION_TIMEOUT 10
#endif

//*****************************************************************************
//
//! Redirection for Ethernet tick handler.
//! Should be called periodically form some interrupt routine as suggested
//
//*****************************************************************************
#define UIEthernetTick( TickMs ) \
	lwIPTimer( TickMs  );

//*****************************************************************************
//
// Functions and data exported by the Ethernet user interface.
//
//*****************************************************************************

extern void 					UIEthernetSendRealTimeData(void);
extern void						UIEthernetInit(tBoolean bUseDHCP);
extern unsigned long			UIEthernetGetIPAddress(void);

#endif // __UI_ETHERNET_H__
