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
//! \file imu.h
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  Inertial Measurement Unit (IMU) API header
//! \warning This API is designed only for Analog Devices IMUs.
//
//*****************************************************************************

#ifndef _IMU_H_
#define _IMU_H_

#include "bsp.h"

//*****************************************************************************
//
//! \addtogroup imu_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Definition for making choice of IMU.
//! \warning Choice of the sensor - remove comment at the particular line.
//! Select \b one and only \b one sensor at the time by defining
//! \b USE_ADIS16405 or \b USE_ADIS16367. Otherwise you will get one and
//! very probably more compilation errors
//
//*****************************************************************************
//#define USE_ADIS16405
#define USE_ADIS16367

//*****************************************************************************
//
// Structures and type definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! \typedef IMUSensorData
//! \brief   Type definition for structure TIMUSensorData
//
//! \struct  TIMUSensorData
//! \brief   Structure for encapsulation of sensor data
//
//! All data item are scaled to Q15 format - for uniqueness.
//! \sa IMUReadSensorData.
//
//*****************************************************************************
typedef struct TIMUSensorData
{
	INT16 SupplyOut;	//!< Power supply measurement value
	
	INT16 XGyro;		//!< X-axis gyroscope output
	INT16 YGyro;		//!< Y-axis gyroscope output
	INT16 ZGyro;		//!< Z-axis gyroscope output
	
	INT16 XAccl;		//!< X-axis accelerometer output
	INT16 YAccl;		//!< Y-axis accelerometer output
	INT16 ZAccl;		//!< Z-axis accelerometer output

#ifdef USE_ADIS16405
	INT16 XMagn;		//!< X-axis magnetometer measurement
	INT16 YMagn;		//!< Y-axis magnetometer measurement
	INT16 ZMagn;		//!< Z-axis magnetometer measurement
	
	INT16 TempOut;		//!< Temperature output
#endif

#ifdef USE_ADIS16367
	INT16 XTemp;		//!< X-axis gyroscope temperature output
	INT16 YTemp;		//!< Y-axis gyroscope temperature output
	INT16 ZTemp;		//!< Z-axis gyroscope temperature output
#endif

	INT16 AuxADC;		//!< Auxiliary ADC output
	
} IMUSensorData;

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Function declarations
//
//*****************************************************************************

UINT16			IMUInit( void );
IMUSensorData*	IMUGetSensorData( void );

#endif /* _IMU_H_ */
