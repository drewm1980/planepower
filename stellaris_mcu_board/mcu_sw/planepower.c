//*****************************************************************************
//
// This file is part of Plane power micro-controller software package.
//
// Plane power micro-controller software package -- A software package for
// interfacing sensors and actuators. Developed by Milan Vukov within the
// Optimization in Engineering Center (OPTEC) under supervision of
// Moritz Diehl. All rights reserved.
//
// Plane power micro-controller software package is free software; you can
// redistribute it and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either version
// 3 of the License, or (at your option) any later version.
//
// Plane power micro-controller software package is distributed in the hope
// that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Plane power micro-controller software package; if not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301  USA
//
//*****************************************************************************

//*****************************************************************************
//
//! \file planepower.c
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  Main application file
//
//*****************************************************************************

#include "bsp.h"

//*****************************************************************************
//
//! \mainpage
//! <h1>Plane power micro-montroller software package</h1>
//!
//! This application is used for interfacing Analog Devices Inertial
//! Measurement Unit (IMU) and RC-servo motors with PC
//! using Ethernet connection and lwIP TCP-IP stack.
//! Main hardware component is EK-LM3S9B92 board by Texas Instruments based on
//! ARM Cortex-M3 micro-controller LM3S9B92.
//!
//! DHCP is used to obtain an Ethernet address. If DHCP times out without
//! obtaining an address, AutoIP will be used to obtain a link-local address.
//! The address that is selected will be shown on the UART.
//!
//! UART0, connected to the FTDI virtual COM port and running at 115,200,
//! 8-N-1, is used to display messages from this application.
//!
//! For additional details on lwIP, refer to the lwIP web page at:
//! http://savannah.nongnu.org/projects/lwip/
//
//*****************************************************************************

//*****************************************************************************
//
//! Main application function
//
//*****************************************************************************
int
main( void )
{
	//
	// BSP (Board Support Package) initialization
	//
	BSPInit();

    //
    // Loop forever.  All the work is done in interrupt handlers.
    //
    while( 1 )
    {
    }
}
