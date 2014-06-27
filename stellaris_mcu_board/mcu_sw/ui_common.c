//*****************************************************************************
//
// ui.c - User interface module.
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

#include "ui_common.h"
#include "ui_commands.h"

//*****************************************************************************
//
//! \page ui_intro User Interface - Introduction
//!
//! There are two user interfaces for the the Brushless DC motor application.
//! One uses an push button for basic control of
//! the motor and two LEDs for basic status feedback, and the other uses the
//! Ethernet port to provide complete control of all aspects of the motor drive
//! as well as monitoring of real-time performance data.
//!
//! The on-board user interface consists of a push button and
//! two LEDs.  The push button cycles between run
//! forward, stop, run backward, stop.
//!
//! The ``Run'' LED flashes the entire time the application is running.  The
//! LED is off most of the time if the motor drive is stopped and on most of
//! the time if it is running.  The ``Fault'' LED is normally off but flashes
//! at a fast rate when a fault occurs.
//!
//! A periodic interrupt is used to poll the state of the push button and
//! perform debouncing.
//!
//! The Ethernet user interface is entirely handled by the Ethernet user
//! interface
//! module.  The only thing provided here is the list of parameters and
//! real-time data items, plus a set of helper functions that are required in
//! order to properly set the values of some of the parameters.
//!
//! This user interface (and the accompanying Ethernet and on-board user
//! interface modules) is more complicated and consumes more program space than
//! would typically exist in a real motor drive application.  The added
//! complexity allows a great deal of flexibility to configure and evaluate the
//! motor drive, its capabilities, and adjust it for the target motor.
//!
//! The code for the user interface is contained in <tt>ui.c</tt>, with
//! <tt>ui.h</tt> containing the definitions for the structures, defines,
//! variables, and functions exported to the remainder of the application.
//
//*****************************************************************************

//*****************************************************************************
//
//! \defgroup ui_common_api User interface - Common Definitions
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! The rate at which the user interface interrupt occurs.
//
//*****************************************************************************
#define UI_INT_RATE             200
#define UI_TICK_MS              (1000/UI_INT_RATE)
#define UI_TICK_US              (1000000/UI_INT_RATE)
#define UI_TICK_NS              (1000000000/UI_INT_RATE)

//*****************************************************************************
//
//! The target type for this drive.  This is used by the user interface
//! module.
//
//*****************************************************************************
const unsigned long g_ulUITargetType = RESP_ID_TARGET_PLANEPOWER;

//*****************************************************************************
//
//! The version of the firmware.  It should only be changed after
//! careful consideration.
//
//*****************************************************************************
const unsigned short g_usFirmwareVersion = 0;

//*****************************************************************************
//
//! An array of structures describing the parameters
//! to the user interface module.
//
//*****************************************************************************

PARAM_TABLE_BEGIN
	//
    // The firmware version.
    //
	PARAM_ITEM( 
		PARAM_FIRMWARE_VERSION,	// Item identification
		2,						// Size in bytes
		0,						// Min
		0,						// Max
		0,						// Step
		g_usFirmwareVersion,	// Variable
		0						// Update handler
	),
	
	
PARAM_TABLE_END

//*****************************************************************************
//
//! An array of structures describing the real-time
//! data items to the user interface module.
//
//*****************************************************************************

UINT16 ui_dummy;

RT_DATA_TABLE_BEGIN
	//
    // TODO: comments
    //
	RT_DATA_ITEM(
		DATA_IMU_SOUT,				// Item identification
		2,							// Size in bytes
		ui_dummy					// Variable
	),
	/*
	RT_DATA_ITEM(
		DATA_IMU_SOUT,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.SupplyOut	// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_XACCL,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.XAccl		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_YACCL,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.YAccl		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_ZACCL,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.ZAccl		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_XGYRO,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.XGyro		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_YGYRO,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.YGyro		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_ZGYRO,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.ZGyro		// Variable
	),

#ifdef USE_ADIS16405
	RT_DATA_ITEM(
		DATA_IMU_XMAGN,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.XMagn		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_YMAGN,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.YMagn		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_ZMAGN,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.ZMagn		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_TOUT,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.TempOut	// Variable
	),
#endif

	RT_DATA_ITEM(
		DATA_IMU_AUXADC,			// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.AuxADC		// Variable
	),
	RT_DATA_ITEM(
		DATA_IMU_SOUT,				// Item identification
		2,							// Size in bytes
		g_sIMUSensorData.SupplyOut	// Variable
	),*/
RT_DATA_TABLE_END


//*****************************************************************************
//
//! The number of real-time data items.  This is used by the
//! user interface module.
//
//*****************************************************************************
const unsigned long g_ulUINumRealTimeData = (sizeof(g_sUIRealTimeData) /
                                             sizeof(g_sUIRealTimeData[0]));

//*****************************************************************************
//
//! The number of parameters.  This is used by the user
//! interface module.
//
//*****************************************************************************
const unsigned long g_ulUINumParameters = (sizeof(g_sUIParameters) /
                                           sizeof(g_sUIParameters[0]));

//*****************************************************************************
//
//! Starts the device.
//!
//! This function is called by the user interface when the run command
//! is received.  The device will be started as a result; this is a no
//! operation if the device is already running.
//!
//! \return None.
//
//*****************************************************************************
void
UIRun( void )
{
    //
    // Start the device.
    //
    // MainRun();
}

//*****************************************************************************
//
//! Stops the device.
//!
//! This function is called by the user interface when the stop command
//! is received.  The device will be stopped as a result; this is a no
//! operation if the device is already stopped.
//!
//! \return None.
//
//*****************************************************************************
void
UIStop(void)
{
    //
    // Stop the motor drive.
    //
    // MainStop();
}

//*****************************************************************************
//
//! Emergency stops the device.
//!
//! This function is called by the serial user interface when the emergency
//! stop command is received.
//!
//! \return None.
//
//*****************************************************************************
void
UIEmergencyStop(void)
{
    //
    // Emergency stop the motor drive.
    //
    // MainEmergencyStop();

    //
    // Indicate that the emergency stop fault has occurred.
    //
    // MainSetFault(FAULT_EMERGENCY_STOP);
}

//*****************************************************************************
//
//! Loads the motor drive parameter block from flash.
//!
//! This function is called by the serial user interface when the load
//! parameter block function is called.  If the device is running, the
//! parameter block is not loaded (since that may result in detrimental
//! changes).
//! If the device is not running and a valid parameter block exists in
//! flash, the contents of the parameter block are loaded from flash.
//!
//! \return None.
//
//*****************************************************************************
void
UIParamLoad(void)
{
	// TODO
}

//*****************************************************************************
//
//! Saves the device parameter block to flash.
//!
//! This function is called by the user interface when the save
//! parameter block function is called.  The parameter block is written to
//! flash for use the next time a load occurs (be it from an explicit request
//! or a power cycle of the device).
//!
//! \return None.
//
//*****************************************************************************
void
UIParamSave(void)
{
	// TODO
}

//*****************************************************************************
//
//! Starts a firmware upgrade.
//!
//! This function is called by the user interface when a firmware
//! upgrade has been requested.  This will branch directly to the boot loader
//! and relinquish all control, never returning.
//!
//! \return None.
//
//*****************************************************************************
void
UIUpgrade(void)
{
    // TODO
}

//*****************************************************************************
//
//! Handles the SysTick interrupt.
//!
//! This function is called when SysTick asserts its interrupt.
//!
//! \return None.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Run the UI Ethernet tick handler.
    //
    UIEthernetTick( UI_TICK_MS );

    //
    // Send real-time data, if appropriate.
    //
    UIEthernetSendRealTimeData();
}

//*****************************************************************************
//
//! Initializes the user interface.
//!
//! This function initializes the user interface modules (ethernet),
//! preparing them to operate.
//!
//! \return None.
//
//*****************************************************************************
void
UIInit(void)
{
	//
	// Initialize the UART.
	//
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioInit(0);
	UARTprintf("\033[2JEK-LM3S9B92 Plane-Power control board\n");
    
    //
    // Initialize the Ethernet user interface. Use DHCP
    //
    UIEthernetInit( true );

    //
    // Configure SysTick to provide a periodic user interface interrupt.
    //
    SysTickPeriodSet( ROM_SysCtlClockGet() / UI_INT_RATE );
    SysTickIntEnable();
    SysTickEnable();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
