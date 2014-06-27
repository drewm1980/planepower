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
//! \file adis16405.h
//! \author Milan Vukov, milan.vukov@esat.kuleuven.be
//! \brief  ADIS16405 Definitions
//
//*****************************************************************************

#ifndef _ADIS16405_H_
#define _ADIS16405_H_

//*****************************************************************************
//
//! \ingroup imu_api
//! \defgroup imu_api_adis16405 ADIS16405 Definitions
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// 		Name            		Address Function
//
//*****************************************************************************

#define ADIS16405_FLASH_CNT		0x00	//!< Flash memory write count              	  
#define ADIS16405_SUPPLY_OUT	0x02	//!< Power supply measurement                 
#define ADIS16405_XGYRO_OUT		0x04	//!< X-axis gyroscope output                  
#define ADIS16405_YGYRO_OUT		0x06	//!< Y-axis gyroscope output                  
#define ADIS16405_ZGYRO_OUT		0x08	//!< Z-axis gyroscope output                  
#define ADIS16405_XACCL_OUT		0x0A	//!< X-axis accelerometer output              
#define ADIS16405_YACCL_OUT		0x0C	//!< Y-axis accelerometer output              
#define ADIS16405_ZACCL_OUT		0x0E	//!< Z-axis accelerometer output              
#define ADIS16405_XMAGN_OUT		0x10	//!< X-axis magnetometer measurement          
#define ADIS16405_YMAGN_OUT		0x12	//!< Y-axis magnetometer measurement          
#define ADIS16405_ZMAGN_OUT		0x14	//!< Z-axis magnetometer measurement          
#define ADIS16405_TEMP_OUT		0x16	//!< Temperature output                       
#define ADIS16405_AUX_ADC		0x18	//!< Auxiliary ADC measurement                
#define ADIS16405_XGYRO_OFF		0x1A	//!< X-axis gyroscope bias offset factor      
#define ADIS16405_YGYRO_OFF		0x1C	//!< Y-axis gyroscope bias offset factor      
#define ADIS16405_ZGYRO_OFF		0x1E	//!< Z-axis gyroscope bias offset factor      
#define ADIS16405_XACCL_OFF		0x20	//!< X-axis acceleration bias offset factor   
#define ADIS16405_YACCL_OFF		0x22	//!< Y-axis acceleration bias offset factor   
#define ADIS16405_ZACCL_OFF		0x24	//!< Z-axis acceleration bias offset factor   
#define ADIS16405_XMAGN_HIF		0x26	//!< X-axis magnetometer, hard-iron factor    
#define ADIS16405_YMAGN_HIF		0x28	//!< Y-axis magnetometer, hard-iron factor    
#define ADIS16405_ZMAGN_HIF		0x2A	//!< Z-axis magnetometer, hard-iron factor    
#define ADIS16405_XMAGN_SIF		0x2C	//!< X-axis magnetometer, soft-iron factor    
#define ADIS16405_YMAGN_SIF		0x2E	//!< Y-axis magnetometer, soft-iron factor    
#define ADIS16405_ZMAGN_SIF		0x30	//!< Z-axis magnetometer, soft-iron factor    
#define ADIS16405_GPIO_CTRL		0x32	//!< Auxiliary digital input/output control   
#define ADIS16405_MSC_CTRL		0x34	//!< Miscellaneous control                    
#define ADIS16405_SMPL_PRD		0x36	//!< Internal sample period (rate) control    
#define ADIS16405_SENS_AVG		0x38	//!< Dynamic range and digital filter contr   
#define ADIS16405_SLP_CNT		0x3A	//!< Sleep mode control                       
#define ADIS16405_DIAG_STAT		0x3C	//!< System status                            
#define ADIS16405_GLOB_CMD		0x3E	//!< System command                           
#define ADIS16405_ALM_MAG1		0x40	//!< Alarm 1 amplitude threshold              
#define ADIS16405_ALM_MAG2		0x42	//!< Alarm 2 amplitude threshold              
#define ADIS16405_ALM_SMPL1		0x44	//!< Alarm 1 sample size                      
#define ADIS16405_ALM_SMPL2		0x46	//!< Alarm 2 sample size                      
#define ADIS16405_ALM_CTRL		0x48	//!< Alarm control                            
#define ADIS16405_AUX_DAC		0x4A	//!< Auxiliary DAC data                       
#define ADIS16405_PRODUCT_ID	0x56	//!< Product identifier, 0x4105

// ADIS16405 specific delays in milliseconds
#define ADIS16405_DELAY_POWER_ON_N		220
#define ADIS16405_DELAY_POWER_ON_LP		290

#define ADIS16405_DELAY_RST_N			100
#define ADIS16405_DELAY_RST_LP			170

#define ADIS16405_DELAY_SLP_REC_N		4
#define ADIS16405_DELAY_SLP_REC_LP		15

#define ADIS16405_DELAY_FLASH_TEST_N	17
#define ADIS16405_DELAY_FLASH_TEST_LP	90

#define ADIS16405_DELAY_SELF_TEST		12

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif /* _ADIS16405_H_ */
