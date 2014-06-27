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
//! \file bsp.c
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  Board Support Package (BSP) API implementation
//
//*****************************************************************************

#include "bsp.h"

#include "imu.h"
#include "rcservo.h"
#include "ui_common.h"

//*****************************************************************************
//
//! \defgroup bsp_api Board Support Package (BSP) API
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! This variable contains number of clock ticks per one microsecond
//! (approximately).
//
//*****************************************************************************
static volatile UINT32 bsp_delay_us_ticks;

//*****************************************************************************
//
//! This variable contains number of clock ticks per one millisecond
//! (approximately).
//
//*****************************************************************************
static volatile UINT32 bsp_delay_ms_ticks;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
//! \brief BSP (Board Support Package) initialization routine.
//! \return 0.
//
//*****************************************************************************
UINT16 BSPInit( void )
{

	//
	// Set the clocking to run from the PLL at 80 MHz.
	//
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	                       SYSCTL_XTAL_16MHZ);
	//
	// Initialize the delay "module"
	//
	BSPDelayInit();

	//
	// Initialize the User Interface Module, including UART0, Ethernet
	// controller and UART stdio (uartstdio) module.
	//
	UIInit();

	//
	// Initialize RC servo motor(s) module together with PWM module
	// and corresponding GPIOs.
	//
	RCServoInit();

	//
	// Initialize IMU (Inertial Measurement Unit) together with
	// SSI0 module
	//
	IMUInit();

	//
	// Set the interrupt priorities
	//
	ROM_IntPrioritySet( INT_GPIOA,		0x00 );
	ROM_IntPrioritySet( FAULT_SYSTICK,	0x40 );
	ROM_IntPrioritySet( INT_ETH,		0x80 );

    //
    // Clean pending interrupts and enable processor interrupts.
    //
	IntPendClear( INT_GPIOA | FAULT_SYSTICK | INT_ETH );
    ROM_IntMasterEnable();

    return 0;
}

//*****************************************************************************
//
//! \brief Initialization routine for delay generation functions.
//! \return 0.
//
//*****************************************************************************
void BSPDelayInit( void )
{
	//
	// According to SysCtlDelay() function, one call to this function
	// takes 3 CPU cycles + function call overhead.
	//
	bsp_delay_us_ticks = ROM_SysCtlClockGet() / 1000000 / 3;
	bsp_delay_ms_ticks = ROM_SysCtlClockGet() / 1000 / 3;
}

//*****************************************************************************
//
//! \brief Microsecond delay routine.
//! \param [in] Delay Delay value in microseconds.
//! \return none.
//
//*****************************************************************************
void BSPDelayUs( UINT32 Delay )
{
	SysCtlDelay( Delay * bsp_delay_us_ticks );
}

//*****************************************************************************
//
//! \brief Millisecond delay routine.
//! \param [in] Delay Delay value in milliseconds.
//! \return none.
//
//*****************************************************************************
void BSPDelayMs( UINT32 Delay )
{
	SysCtlDelay( Delay * bsp_delay_ms_ticks );
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
