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
//! \file bsp.h
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  Board Support Package (BSP) API header
//
//*****************************************************************************

#ifndef _BSP_H_
#define _BSP_H_

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"

#include "driverlib/ethernet.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/ssi.h"
#include "driverlib/pwm.h"

#include "utils/locator.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"

//*****************************************************************************
//
//! \addtogroup bsp_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// Defines of custom types.
//
//*****************************************************************************
#define UINT8	unsigned char	//!< Type definition of unsigned 8-bit number
#define UINT16	unsigned short	//!< Type definition of unsigned 16-bit number
#define UINT32	unsigned long	//!< Type definition of unsigned 32-bit number

#define INT8	char			//!< Type definition of 8-bit number
#define INT16	short			//!< Type definition of 16-bit number
#define INT32	long			//!< Type definition of 32-bit number

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Functions exported by the Board Support Package.
//
//*****************************************************************************

extern UINT16	BSPInit( void );

extern void		BSPDelayInit( void );
extern void		BSPDelayUs( UINT32 Delay );
extern void		BSPDelayMs( UINT32 Delay );

#endif /* _BSP_H_ */
