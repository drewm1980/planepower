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
//! \file   rcservo.c
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  RC-Servo API implementation
//
//*****************************************************************************

#include "rcservo.h"

//*****************************************************************************
//
//! \defgroup rcservo_api RC-Servo API
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! RC-servo minimum pulse width
//
//*****************************************************************************
#define RC_DUTY_WIDTH_MIN	1125

//*****************************************************************************
//
//! RC-servo mean pulse width
//
//*****************************************************************************
#define RC_DUTY_WIDTH_MEAN	1875

//*****************************************************************************
//
//! RC-servo maximum pulse width
//
//*****************************************************************************
#define RC_DUTY_WIDTH_MAX	2625

//*****************************************************************************
//
//! Macro for calculating: ( RC_DUTY_WIDTH_MAX - RC_DUTY_WIDTH_MIN ) / 2.
//
//*****************************************************************************
#define RC_DUTY_WIDTH_HALF	( ( RC_DUTY_WIDTH_MAX - RC_DUTY_WIDTH_MIN ) >> 1 )

//*****************************************************************************
//
//! Macro for scaling the input Value in Q15 format to range
//! RC_DUTY_WIDTH_MIN - RC_DUTY_WIDTH_MAX.
//
//*****************************************************************************
#define RC_DUTY_SCALE( Value ) \
	(UINT16)((((INT32) RC_DUTY_WIDTH_HALF * Value ) >> 15 ) + RC_DUTY_WIDTH_MEAN )

//*****************************************************************************
//
//! \brief  Initialization function for PWM module and corresponding GPIO pins
//
//! CPU frequency: 80 MHz
//! PWM input clock prescaler value: 64
//!
//! --->
//!
//! PWM output clock period: 1 / ( 80 MHz / 64 ) = 0.0125 usec * 64 = 0.8 usec ( = T_PWMGEN )
//!
//! RC Servos by HITEC:
//! PWM Period:       20  msec <-> 20000 / 0.8 = 25000 PWM clock cycles
//!
//! Min  pulse width: 0.9 msec <-> 0.9 * 25000 = 1125 <=> RC_DUTY_WIDTH_MIN
//!
//! Mean pulse width: 1.5 msec <-> 1.5 * 25000 = 1875 <=> RC_DUTY_WIDTH_MEAN
//!
//! Max  pulse width: 2.1 msec <-> 2.1 * 25000 = 2625 <=> RC_DUTY_WIDTH_MAX
//
//! \return 0
//
//*****************************************************************************
UINT16 RCServoInit( void )
{
	UINT32 Period;

	//
	// Enable the peripherals used by this example.
	//
	ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_PWM );
	ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOD );

	//
	// Set GPIO D0, D1 and D2 as PWM pins.  They are used to output the PWM0,
	// PWM1 and PWM2 signals.
	//
	GPIOPinConfigure( GPIO_PD0_PWM0 );
	GPIOPinConfigure( GPIO_PD1_PWM1 );
	GPIOPinConfigure( GPIO_PD2_PWM2 );
	ROM_GPIOPinTypePWM( GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 );

	//
	// Configure the PWM module clock. In this case we assume that CPU master
	// clock is 80 MHz and PWM clock is 80/64 = 1.25 MHz. This is the lowest
	// frequency and we need it in order to make PWM period of 20 msec.
	//
	ROM_SysCtlPWMClockSet( SYSCTL_PWMDIV_64 );

	//
	// For PWM clock of 1.25 MHz we need 25000 counts for 20 msec period.
	//
	Period = 25000;

	//
	// Configure PWM generators and set the initial PWM periods to neutral.
	//
	ROM_PWMGenConfigure(
			PWM_BASE,
			PWM_GEN_0,
			PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC
	);
	ROM_PWMGenConfigure(
			PWM_BASE,
			PWM_GEN_1,
			PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC
	);
	ROM_PWMGenPeriodSet( PWM_BASE, PWM_GEN_0, Period );
	ROM_PWMGenPeriodSet( PWM_BASE, PWM_GEN_1, Period );

	//
	// Set PWM0, PWM1 and PWM2 pulse widths to 'neutral'.
	//
	ROM_PWMPulseWidthSet( PWM_BASE, PWM_OUT_0, RC_DUTY_WIDTH_MEAN  );
	ROM_PWMPulseWidthSet( PWM_BASE, PWM_OUT_1, RC_DUTY_WIDTH_MEAN );
	ROM_PWMPulseWidthSet( PWM_BASE, PWM_OUT_2, RC_DUTY_WIDTH_MEAN );

	//
	// Enable the PWM0, PWM1 and PWM2 output signals.
	//
	ROM_PWMOutputState( PWM_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT | PWM_OUT_2_BIT, true );

	//
	// Enable the PWM generators.
	//
	ROM_PWMGenEnable( PWM_BASE, PWM_GEN_0 );
	ROM_PWMGenEnable( PWM_BASE, PWM_GEN_1 );

	return 0;
}

//*****************************************************************************
//
//! \brief Function for updating position references of all three RC-servos.
//
//! All three input parameters are expected in Q15 format - signed 16-bit
//! numbers
//
//! \param [in] Servo1Duty Position reference for motor 1.
//! \param [in] Servo2Duty Position reference for motor 2.
//! \param [in] Servo3Duty Position reference for motor 3.
//! \return 0.
//
//*****************************************************************************
UINT16 RCServoUpdate( INT16 Servo1Duty, INT16 Servo2Duty, INT16 Servo3Duty )
{
	//
	// Update the duty cycles
	//
	PWMPulseWidthSet( PWM_BASE, PWM_OUT_0, RC_DUTY_SCALE( Servo1Duty ) );
	PWMPulseWidthSet( PWM_BASE, PWM_OUT_1, RC_DUTY_SCALE( Servo2Duty ) );
	PWMPulseWidthSet( PWM_BASE, PWM_OUT_2, RC_DUTY_SCALE( Servo3Duty ) );

	//
	// Perform a synchronous update of the PWM generators.
	//
	PWMSyncUpdate( PWM_BASE, PWM_GEN_0_BIT | PWM_GEN_1_BIT );

	return 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
