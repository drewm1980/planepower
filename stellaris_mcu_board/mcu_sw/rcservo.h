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
//! \file   rcservo.h
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  RC-Servo API header
//
//*****************************************************************************

#ifndef _RCSERVO_H_
#define _RCSERVO_H_

#include "bsp.h"

extern UINT16 RCServoInit( void );
extern UINT16 RCServoUpdate( INT16 Servo1Duty, INT16 Servo2Duty, INT16 Servo3Duty );

#endif /* _RCSERVO_H_ */
