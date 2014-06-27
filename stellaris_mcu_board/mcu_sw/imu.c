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
//! \file imu.c
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  Inertial Measurement Unit (IMU) API implementation
//! \warning This API is designed only for Analog Devices IMUs.
//
//*****************************************************************************

#include "imu.h"

//*****************************************************************************
//
//! \defgroup imu_api Inertial Measurement Unit (IMU) API
//! @{
//
//*****************************************************************************

#if defined( USE_ADIS16405 ) && defined( USE_ADIS16367 )
	#error "You can use ONE and only ONE sensor at the time!"
#endif

#if defined( USE_ADIS16405 )
	#include "adis16405.h"

	//
	//! Definition of the IMU Global command register
	//
	#define IMU_GLOB_CMD ADIS16405_GLOB_CMD
#elif defined( USE_ADIS16367 )
	#include "adis16367.h"

	//
	//! Definition of the IMU Global command register
	//
	#define IMU_GLOB_CMD ADIS16367_GLOB_CMD
#else
	#error "None of the sensors is selected!"
#endif


//*****************************************************************************
//
// Peripheral definitions for THE board and IMU related stuff
//
//*****************************************************************************

// SSI port
#define IMU_SSI_BASE            SSI0_BASE
#define IMU_SSI_SYSCTL_PERIPH   SYSCTL_PERIPH_SSI0

// GPIO for SSI pins
#define IMU_GPIO_PORT_BASE      GPIO_PORTA_BASE
#define IMU_GPIO_SYSCTL_PERIPH  SYSCTL_PERIPH_GPIOA
#define IMU_SSI_CLK             GPIO_PIN_2
#define IMU_SSI_TX              GPIO_PIN_5
#define IMU_SSI_RX              GPIO_PIN_4

// GPIO for IMU chip select, interrupt and reset pins
#define IMU_CS					GPIO_PIN_3				// Active low
#define IMU_INT					GPIO_PIN_6				// Active high
#define IMU_RST					GPIO_PIN_7				// Active low

//*****************************************************************************
//
//! IMU SSI frequency in Hz
//
//*****************************************************************************
#define IMU_SSI_FREQUENCY		1000000

//*****************************************************************************
//
//! IMU regular delay between regular transfers in microseconds.
//
//*****************************************************************************
#define IMU_READ_DELAY_US		9

//*****************************************************************************
//
//! IMU regular delay between transfers in burst mode in microseconds.
//! \note Minimal value is 1/Fsclk, Fsclk is SSI clock frequency.
//
//*****************************************************************************
#define IMU_READ_DELAY_BURST_US	1

//*****************************************************************************
//
//! IMU chip select (CS) enable macro
//
//*****************************************************************************
#define IMU_CS_ENABLE()		\
	GPIOPinWrite( IMU_GPIO_PORT_BASE, IMU_CS, 0 );

//*****************************************************************************
//
//! IMU chip select (CS) disable macro
//
//*****************************************************************************
#define IMU_CS_DISABLE()	\
	GPIOPinWrite( IMU_GPIO_PORT_BASE, IMU_CS, IMU_CS );

//*****************************************************************************
//
//! IMU read command selection
//
//*****************************************************************************
#define IMU_CMD_READ			0x0000

//*****************************************************************************
//
//! IMU write command selection
//
//*****************************************************************************
#define IMU_CMD_WRITE			0x8000

//*****************************************************************************
//
//! Dummy (helper) variable.
//
//*****************************************************************************
static UINT32 					imu_dummy;

//*****************************************************************************
//
//! Read pointer to IMU datastructure with most fresh data ready for reading
//! \sa IMUReadSensorData
//
//*****************************************************************************
static volatile IMUSensorData*	imu_read;

//*****************************************************************************
//
//! Write pointer to IMU data structure in which data should be put.
//! \sa IMUReadSensorData
//
//*****************************************************************************
static volatile IMUSensorData*	imu_write;

//*****************************************************************************
//
//! Ping data buffer for placing sensor data.
//
//*****************************************************************************
static volatile IMUSensorData	imu_ping_buffer;

//*****************************************************************************
//
//! Pong data buffer for placing sensor data.
//
//*****************************************************************************
static volatile IMUSensorData	imu_pong_buffer;

//*****************************************************************************
//
//! \brief Handy macro definition for sending and receiving data from IMU
//! \param [in] rw Read/Write selection, use \b IMU_CMD_READ for read and
//!                \b IMU_CMD_WRITE for write
//! \param [in] command Command which you want to send
//! \param [in] dataout Data which you want to send to the IMU
//! \param [out] datain Data which you receive from IMU as a response
//
//*****************************************************************************
#define IMU_RXTX( rw, command, dataout, datain ) \
	SSIDataPut( IMU_SSI_BASE, (UINT16)rw + \
		( ( (UINT16)(command) & 0x007F) << 8 ) + ( (UINT16)dataout & 0x00FF ) \
	); \
	SSIDataGet( IMU_SSI_BASE, &(imu_dummy) ); \
	datain = (UINT16) imu_dummy;

//*****************************************************************************
//
//! \brief IMU Initialization routine.
//
//! This routine initializes SSI module of the MCU as well as corresponding
//! GPIO pins. After configuration of SSI and GPIO modules, IMU is then
//! prepared for normal operation. Have in mind that some initialization is
//! is IMU specific. At the end, we perform configuration of GPIO interrupt
//! routine IMUIntHandler - IMU sends the interrupt when data is ready for read.
//! That interrupt is used to trigger the MCU to read the data.
//
//! \return 0.
//
//*****************************************************************************
UINT16 IMUInit( void )
{
	//
	// Enable the peripherals used to drive the IMU on SSI, and the CS.
	//
    SysCtlPeripheralEnable( IMU_SSI_SYSCTL_PERIPH );
    SysCtlPeripheralEnable( IMU_GPIO_SYSCTL_PERIPH );

    //
    // Configure the appropriate pins to be SSI instead of GPIO.
    //
    GPIOPinConfigure( GPIO_PA2_SSI0CLK );
    GPIOPinConfigure( GPIO_PA4_SSI0RX );
    GPIOPinConfigure( GPIO_PA5_SSI0TX );
    ROM_GPIOPinTypeSSI( IMU_GPIO_PORT_BASE, IMU_SSI_CLK | IMU_SSI_RX | IMU_SSI_TX );
    GPIOPadConfigSet( IMU_GPIO_PORT_BASE, IMU_SSI_CLK | IMU_SSI_TX | IMU_SSI_RX,
    	GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );

    //
    // Configure chip select and reset signals as classic GPIO outputs.
    // After configuration set both pins HIGH.
    //
   	GPIOPinTypeGPIOOutput( IMU_GPIO_PORT_BASE, IMU_CS | IMU_RST );
   	GPIOPadConfigSet( IMU_GPIO_PORT_BASE, IMU_CS | IMU_RST,
     	GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );
   	IMU_CS_DISABLE();
   	GPIOPinWrite( IMU_GPIO_PORT_BASE, IMU_RST, IMU_RST );

   	//
    // Configure the SSI0 port and enable it.
   	//
    SSIConfigSetExpClk(
    	IMU_SSI_BASE,
		SysCtlClockGet(),
        SSI_FRF_MOTO_MODE_3,	// Table 6, <DP10>
        SSI_MODE_MASTER,
        IMU_SSI_FREQUENCY,		// For burst mode, <DP10>
        16						// Table 7, <DP10>
	);
	SSIEnable( IMU_SSI_BASE );

#ifdef USE_ADIS16405
	//
	// Wait, Reset, Wait, Soft Reset, Wait
	//
	BSPDelayMs( ADIS16405_DELAY_POWER_ON_N );
    GPIOPinWrite( IMU_GPIO_PORT_BASE, IMU_RST, 0 );
    BSPDelayUs( 500 );
	GPIOPinWrite( IMU_GPIO_PORT_BASE, IMU_RST, IMU_RST );
    BSPDelayMs( ADIS16405_DELAY_RST_N );

    //
    // Check for the anomaly - interrupts should be generated by default
    // but on some parts this is not the case.
    //
    IMU_CS_ENABLE( );
    IMU_RXTX( IMU_CMD_READ, ADIS16405_MSC_CTRL, 0x00, imu_dummy );
    IMU_CS_DISABLE();
    BSPDelayUs( IMU_READ_DELAY_US );
    if ( imu_dummy == 0x00 )
    {
    	BSPDelayMs( 500  );

    	IMU_CS_ENABLE( );
    	IMU_RXTX( IMU_CMD_WRITE, ADIS16405_MSC_CTRL, 0x06, imu_dummy );
    	IMU_CS_DISABLE();
    	BSPDelayUs( IMU_READ_DELAY_US );

    	IMU_CS_ENABLE( );
    	IMU_RXTX( IMU_CMD_WRITE, ADIS16405_GLOB_CMD, 0x08, imu_dummy );
    	IMU_CS_DISABLE();

    	BSPDelayMs( 100 );
    }
#endif

#ifdef USE_ADIS16367
    //
	// Wait, Reset, Wait, Soft Reset, Wait
    //
	BSPDelayMs( ADIS16367_DELAY_POWER_ON_N );
    GPIOPinWrite( IMU_GPIO_PORT_BASE, IMU_RST, 0 );
    BSPDelayUs( 500 );
	GPIOPinWrite( IMU_GPIO_PORT_BASE, IMU_RST, IMU_RST );
    BSPDelayMs( ADIS16367_DELAY_RST_N );
#endif

    //
    // Initialize read and write pointers
    //
    imu_read	= &imu_ping_buffer;
    imu_write	= &imu_pong_buffer;

    //
	// Configure IMU interrupt as classic GPIO input
    //
	GPIOPinTypeGPIOInput( IMU_GPIO_PORT_BASE, IMU_INT );
	GPIOPadConfigSet( IMU_GPIO_PORT_BASE, IMU_INT,
		GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD );
	GPIOIntTypeSet( IMU_GPIO_PORT_BASE, IMU_INT, GPIO_FALLING_EDGE );
	GPIOPinIntClear( IMU_GPIO_PORT_BASE, IMU_INT );
	GPIOPinIntEnable( IMU_GPIO_PORT_BASE, IMU_INT );
	IntEnable( INT_GPIOA );

	return 0;
}

//*****************************************************************************
//
//! \brief Function for reading sensor data via SSI interface.
//! \note This function is not interrupt/thread safe, so it must be called
//! from high(est) priority interrupt routine.
//! \sa IMUIntHandler.
//! \return Pointer to the structure IMUSensorData which contains fresh sensor
//! data.
//
//*****************************************************************************
static IMUSensorData* IMUReadSensorData( void )
{
	//
	// TODO: Code checking of ND and EA bits, <DP12>.
	//

	//
	// Enable IMU Chip select
	//
	IMU_CS_ENABLE();

	//
	// Initialize BURST mode, and read first value, SupplyOut
	// and send some dummy cmd & data to receive all other sensor values
	//
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_dummy ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->SupplyOut ); BSPDelayUs( IMU_READ_DELAY_BURST_US );

	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->XGyro ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->YGyro ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->ZGyro ); BSPDelayUs( IMU_READ_DELAY_BURST_US );

	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->XAccl ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->YAccl ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->ZAccl ); BSPDelayUs( IMU_READ_DELAY_BURST_US );

#ifdef USE_ADIS16405
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->XMagn ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->YMagn ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->ZMagn ); BSPDelayUs( IMU_READ_DELAY_BURST_US );

	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->TempOut ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
#endif

#ifdef USE_ADIS16367
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->XTemp ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->YTemp ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
	IMU_RXTX( IMU_CMD_READ, IMU_GLOB_CMD, 0x00, imu_write->ZTemp ); BSPDelayUs( IMU_READ_DELAY_BURST_US );
#endif

	//
	// The last is on the FIFO and we would like to get it from the FIFO
	// TODO: Check this once more...
	//
	SSIDataGetNonBlocking( IMU_SSI_BASE, &imu_dummy );
	imu_write->AuxADC = (UINT16) imu_dummy;

	//
	// Disable IMU chip select
	//
	IMU_CS_DISABLE();

	//
	// Perform some scaling. After this scaling all values will be in Q15
	// format: 0 <-> 0x0000, cca 1 <-> 0x7FFF, -1 <-> 0x8000.
	//
	imu_write->SupplyOut <<= 2;

	imu_write->XGyro <<= 2;
	imu_write->YGyro <<= 2;
	imu_write->ZGyro <<= 2;

	imu_write->XAccl <<= 2;
	imu_write->YAccl <<= 2;
	imu_write->ZAccl <<= 2;

#ifdef USE_ADIS16405
	imu_write->XMagn <<= 2;
	imu_write->YMagn <<= 2;
	imu_write->ZMagn <<= 2;

	imu_write->TempOut <<= 4;
#endif

#ifdef USE_ADIS16367
	imu_write->XTemp <<= 4;
	imu_write->YTemp <<= 4;
	imu_write->ZTemp <<= 4;
#endif

	imu_write->AuxADC  <<= 4;

	//
	// Exchange read and write pointers
	//
	if ( imu_read == &imu_ping_buffer )
	{
		imu_read	= &imu_pong_buffer;
		imu_write	= &imu_ping_buffer;
	}
	else
	{
		imu_read	= &imu_ping_buffer;
		imu_write	= &imu_pong_buffer;
	}

	//
	// And return the pointer to the read-ready structure with
	// most fresh sensor data. Type-casting is needed to avoid compiler warnings
	//
	return (IMUSensorData*)imu_read;
}

//*****************************************************************************
//
//! \brief IMU GPIO handler routine.
//
//! Interrupt routine which is responsible for reading sensor data.
//! Interrupt is generated by IMU pin DIO1 which is connected to MCU pin
//! GPIO_PA6 (IMU_INT).
//
//*****************************************************************************
void IMUIntHandler( void )
{
	//
	// Clear the pin interrupt.
	//
	ROM_GPIOPinIntClear( IMU_GPIO_PORT_BASE, IMU_INT );

	//
	// Read the IMU sensor data.
	// Return value of function IMUReadSensorData is just neglected.
	//
	IMUReadSensorData();
}

//*****************************************************************************
//
//! \brief Function which returns a pointer to the structure with sensor data.
//
//! If someone wants to access sensor data outside this (IMU) module, he should
//! call this function to get the pointer to the structure with current sensor
//! data.
//
//*****************************************************************************
IMUSensorData* IMUGetSensorData( void )
{
	//
	// Type-casting is needed to avoid compiler warnings
	//
	return (IMUSensorData*)imu_read;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
