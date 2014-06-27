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
//! \file adis16367.h
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  ADIS16367 Definitions
//
//*****************************************************************************

#ifndef _ADIS16367_H_
#define _ADIS16367_H_

//*****************************************************************************
//
//! \ingroup imu_api
//! \defgroup imu_api_adis16367 ADIS16367 Definitions
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//      Name					Address  	Register Description
//
//*****************************************************************************

#define ADIS16367_FLASH_CNT		0x00		//!< Flash memory write count               	
#define ADIS16367_SUPPLY_OUT	0x02      	//!< Power supply measurement                	
#define ADIS16367_XGYRO_OUT		0x04      	//!< X-axis gyroscope output                 	
#define ADIS16367_YGYRO_OUT		0x06      	//!< Y-axis gyroscope output                 	
#define ADIS16367_ZGYRO_OUT		0x08      	//!< Z-axis gyroscope output                 	
#define ADIS16367_XACCL_OUT		0x0A      	//!< X-axis accelerometer output             	
#define ADIS16367_YACCL_OUT		0x0C      	//!< Y-axis accelerometer output             	
#define ADIS16367_ZACCL_OUT		0x0E      	//!< Z-axis accelerometer output             	
#define ADIS16367_XTEMP_OUT		0x10      	//!< X-axis gyroscope temperature output     	
#define ADIS16367_YTEMP_OUT		0x12      	//!< Y-axis gyroscope temperature output     	
#define ADIS16367_ZTEMP_OUT		0x14      	//!< Z-axis gyroscope temperature output     	
#define ADIS16367_AUX_ADC		0x16      	//!< Auxiliary ADC output                    	
#define ADIS16367_XGYRO_OFF		0x1A      	//!< X-axis gyroscope bias offset factor     	
#define ADIS16367_YGYRO_OFF		0x1C      	//!< Y-axis gyroscope bias offset factor     	
#define ADIS16367_ZGYRO_OFF		0x1E      	//!< Z-axis gyroscope bias offset factor     	
#define ADIS16367_XACCL_OFF		0x20      	//!< X-axis acceleration bias offset factor  	
#define ADIS16367_YACCL_OFF		0x22      	//!< Y-axis acceleration bias offset factor  	
#define ADIS16367_ZACCL_OFF		0x24		//!< Z-axis acceleration bias offset factor  	
#define ADIS16367_ALM_MAG1		0x26      	//!< Alarm 1 amplitude threshold             	
#define ADIS16367_ALM_MAG2		0x28      	//!< Alarm 2 amplitude threshold             	
#define ADIS16367_ALM_SMPL1		0x2A      	//!< Alarm 1 sample size                     	
#define ADIS16367_ALM_SMPL2		0x2C      	//!< Alarm 2 sample size                     	
#define ADIS16367_ALM_CTRL		0x2E      	//!< Alarm control                           	
#define ADIS16367_AUX_DAC		0x30      	//!< Auxiliary DAC data                      	
#define ADIS16367_GPIO_CTRL		0x32      	//!< Auxiliary digital input/output control   
#define ADIS16367_MSC_CTRL		0x34      	//!< Data-ready, self-test, miscellaneous     
#define ADIS16367_SMPL_PRD		0x36      	//!< Internal sample period (rate) control    
#define ADIS16367_SENS_AVG		0x38      	//!< Dynamic range and digital filter control 
#define ADIS16367_SLP_CNT		0x3A      	//!< Sleep mode control                       
#define ADIS16367_DIAG_STAT		0x3C      	//!< System status                            
#define ADIS16367_GLOB_CMD		0x3E      	//!< System command                           
#define ADIS16367_LOT_ID1		0x52      	//!< Lot Identification Code 1                
#define ADIS16367_LOT_ID2		0x54      	//!< Lot Identification Code 2                
#define ADIS16367_PROD_ID		0x56      	//!< Product identification, 0x3FEF           
#define ADIS16367_SERIAL_NUM	0x58      	//!< Serial number                            

// ADIS16367 specific delays in milliseconds
#define ADIS16367_DELAY_POWER_ON_N		180
#define ADIS16367_DELAY_POWER_ON_LP		250

#define ADIS16367_DELAY_RST_N			60
#define ADIS16367_DELAY_RST_LP			130

#define ADIS16367_DELAY_SLP_REC_N		4
#define ADIS16367_DELAY_SLP_REC_LP		9

#define ADIS16367_DELAY_FLASH_TEST_N	17
#define ADIS16367_DELAY_FLASH_TEST_LP	90

#define ADIS16367_DELAY_SELF_TEST		12

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif /* _ADIS16367_H_ */
