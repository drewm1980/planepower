//*****************************************************************************
//
// ui_commands.h - Definitions used by the user interface communication 
//                 protocol.
//
//*****************************************************************************
//
// Copyright (c) 2006-2011 Texas Instruments Incorporated.  All rights reserved.
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

//*****************************************************************************
//
//! \defgroup ui_commands_api User Interface - Commands
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! \defgroup ui_commands_api_tags Tags
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! The value of the <tt>{tag}</tt> byte for a command packet.
//
//*****************************************************************************
#define TAG_CMD        0xff

//*****************************************************************************
//
//! The value of the <tt>{tag}</tt> byte for a status packet.
//
//*****************************************************************************
#define TAG_STATUS     0xfe

//*****************************************************************************
//
//! The value of the <tt>{tag}</tt> byte for a real-time data packet.
//
//*****************************************************************************
#define TAG_DATA       0xfd

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
//! \defgroup ui_commands_api_commands Commands and responses
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! This command is used to determine the type of motor driven by the board.
//! In this context, the type of motor is a broad statement; for example,
//! both single-phase and three-phase AC induction motors can be driven by a
//! single AC induction motor board (not simultaneously, of course).
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_ID_TARGET {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x05 CMD_ID_TARGET {type} {checksum}
//! \endcode
//!
//! - <tt>{type}</tt> identifies the motor drive type; will be one of
//! #RESP_ID_TARGET_BLDC, #RESP_ID_TARGET_STEPPER, or #RESP_ID_TARGET_ACIM.
//
//*****************************************************************************
#define CMD_ID_TARGET           0x00

//*****************************************************************************
//
//! The response returned by the #CMD_ID_TARGET command for a planepower SW
//
//*****************************************************************************
#define RESP_ID_TARGET_PLANEPOWER	0x00

//*****************************************************************************
//
//! Starts an upgrade of the firmware on the target. There is no response
//! to this command; once received, the target will return to the control
//! of the Stellaris boot loader and its serial protocol.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_UPGRADE {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     <none>
//! \endcode
//
//*****************************************************************************
#define CMD_UPGRADE             0x01

//*****************************************************************************
//
//! This command is used to discover the motor drive board(s) that may be
//! connected to the networked communication channel (e.g. CAN, Ethernet).
//! This command is similar to the CMD_ID_TARGET command, but intended for
//! networked operation.  Additional parameters are available in the response
//! that will allow the networked device to provide board-specific information
//! (e.g. configuration switch settings) that can be used to identify which
//! board is to be selected for operation.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_DISCOVER_TARGET {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x0A CMD_DISCOVER_TARGET {type} {id} {remote-ip} {checksum}
//! \endcode
//!
//! - <tt>{type}</tt> identifies the motor drive type; will be one of
//! #RESP_ID_TARGET_BLDC, #RESP_ID_TARGET_STEPPER, or #RESP_ID_TARGET_ACIM.
//! - <tt>{id}</tt> is a board-specific identification value; will typically
//! be the setting read from a set of configuration switches on the board.
//! - <tt>{config}</tt> is used to provide additional (if needed) board
//! configuration information.  The interpretation of this field will vary
//! with the board type.
//
//*****************************************************************************
#define CMD_DISCOVER_TARGET     0x02

//*****************************************************************************
//
//! Gets a list of the parameters supported by this motor drive.  This
//! command returns a list of parameter numbers, in no particular order;
//! each will be one of the \b PARAM_xxx values.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_GET_PARAMS {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS {length} CMD_GET_PARAMS {param} [{param} ...] {checksum}
//! \endcode
//!
//! - <tt>{param}</tt> is a list of one or more \b PARAM_xxx values.
//
//*****************************************************************************
#define CMD_GET_PARAMS          0x10

//*****************************************************************************
//
//! Gets the description of a parameter.  The size of the parameter value,
//! the minimum and maximum values for the parameter, and the step between
//! valid values for the parameter.  If the minimum, maximum, and step
//! values don't make sense for a parameter, they may be omitted from
//! the response, leaving only the size.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x05 CMD_GET_PARAM_DESC {param} {checksum}
//! \endcode
//!
//! - <tt>{param}</tt> is one of the \b PARAM_xxx values.
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS {length} CMD_GET_PARAM_DESC {size} {min} [{min} ...]
//!         {max} [{max} ...] {step} [{step} ...] {checksum}
//! \endcode
//!
//! - <tt>{size}</tt> is the size of the parameter in bytes.
//! - <tt>{min}</tt> is the minimum valid value for this parameter.  The number
//!   of bytes for this value is determined by the size of the parameter.
//! - <tt>{max}</tt> is the maximum valid value for this parameter.  The number
//!   of bytes for this value is determined by the size of the parameter.
//! - <tt>{step}</tt> is the increment between valid values for this parameter.
//!   It should be the case that ``min + (step * N) = max'' for some positive
//!   integer N.  The number of bytes for this value is determined by the size
//!   of the parameter.
//
//*****************************************************************************
#define CMD_GET_PARAM_DESC      0x11

//*****************************************************************************
//
//! Gets the value of a parameter.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x05 CMD_GET_PARAM_VALUE {param} {checksum}
//! \endcode
//!
//! - <tt>{param}</tt> is the parameter whose value should be returned; must
//!   be one of the parameters returned by #CMD_GET_PARAMS.
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS {length} CMD_GET_PARAM_VALUE {value} [{value} ...]
//!         {checksum}
//! \endcode
//!
//! - <tt>{value}</tt> is the current value of the parameter.  All bytes of the
//!   value will always be returned.
//
//*****************************************************************************
#define CMD_GET_PARAM_VALUE     0x12

//*****************************************************************************
//
//! Sets the value of a parameter.  For parameters that have values larger
//! than a single byte, not all bytes of the parameter value need to be
//! supplied; value bytes that are not supplied (that is, the more
//! significant bytes) are treated as if a zero was transmitted.  If more bytes
//! than required for the parameter value are supplied, the extra bytes are
//! ignored.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD {length} CMD_SET_PARAM_VALUE {param} {value} [{value} ...]
//!         {checksum}
//! \endcode
//!
//! - <tt>{param}</tt> is the parameter whose value should be set; must be one
//!   of the parameters returned by #CMD_GET_PARAMS.
//! - <tt>{value}</tt> is the new value for the parameter.
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_SET_PARAM_VALUE {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_SET_PARAM_VALUE     0x13

//*****************************************************************************
//
//! Loads the most recent parameter set from flash, causing the current
//! parameter values to be lost.  This can be used to recover from parameter
//! changes that do not work very well.  For example, if a set of parameter
//! changes are made during experimentation and they turn out to cause the
//! motor to perform poorly, this will restore the last-saved
//! parameter set (which is presumably, but not necessarily, of better
//! quality).
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_LOAD_PARAMS {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_LOAD_PARAMS {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_LOAD_PARAMS         0x14

//*****************************************************************************
//
//! Saves the current parameter set to flash.  Only the most recently
//! saved parameter set is available for use, and it contains the
//! default settings of all the parameters at power-up.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_SAVE_PARAMS {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_SAVE_PARAMS {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_SAVE_PARAMS         0x15

//*****************************************************************************
//
//! Gets a list of the real-time data items supported by this motor drive.
//! This command returns a list of real-time data item numbers, in no
//! particular order, along with the size of the data item; each
//! data item will be one of the \b DATA_xxx values.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_GET_DATA_ITEMS {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS {length} CMD_GET_DATA_ITEMS {item} {size}
//!         [{item} {size} ...] {checksum}
//! \endcode
//!
//! - <tt>{item}</tt> is a list of one or more \b DATA_xxx values.
//! - <tt>{size}</tt> is the size of the data item immediately preceding.
//
//*****************************************************************************
#define CMD_GET_DATA_ITEMS      0x20

//*****************************************************************************
//
//! Adds a real-time data item to the real-time data output stream.  To
//! avoid a change in the real-time data output stream at an unexpected
//! time, this command should only be issued when the real-time data output
//! stream is disabled.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x05 CMD_ENABLE_DATA_ITEM {item} {checksum}
//! \endcode
//!
//! - <tt>{item}</tt> is the real-time data item to be added to the real-time
//!   data output stream; must be one of the \b DATA_xxx values.
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_ENABLE_DATA_ITEM {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_ENABLE_DATA_ITEM    0x21

//*****************************************************************************
//
//! Removes a real-time data item from the real-time data output
//! stream.  To avoid a change in the real-time data output stream at an
//! unexpected time, this command should only be issued when the real-time
//! data output stream is disabled.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x05 CMD_DISABLE_DATA_ITEM {item} {checksum}
//! \endcode
//!
//! - <tt>{item}</tt> is the real-time data item to be removed from the
//!   real-time data output stream; must be one of the \b DATA_xxx values.
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_DISABLE_DATA_ITEM {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_DISABLE_DATA_ITEM   0x22

//*****************************************************************************
//
//! Starts the real-time data output stream.  Only those values that have
//! been added to the output stream will be provided, and it will continue
//! to run (regardless of any other motor drive state) until stopped.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_START_DATA_STREAM {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_START_DATA_STREAM {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_START_DATA_STREAM   0x23

//*****************************************************************************
//
//! Stops the real-time data output stream.  The output stream should be
//! stopped before real-time data items are added to or removed from the
//! stream to avoid unexpected changes in the stream data (it will all
//! be valid data, there is simply no easy way to know what real-time data
//! items are in a #TAG_DATA packet if changes are made while the output stream
//! is running).
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_STOP_DATA_STREAM {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_STOP_DATA_STREAM {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_STOP_DATA_STREAM    0x24

//*****************************************************************************
//
//! Starts the motor running based on the current parameter set, if it is
//! not already running.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_RUN {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_RUN {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_RUN                 0x30

//*****************************************************************************
//
//! Stops the motor, if it is not already stopped.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_STOP {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_STOP {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_STOP                0x31

//*****************************************************************************
//
//! Stops the motor, if it is not already stopped.  This may take more
//! aggressive action than #CMD_STOP at the cost of precision.  For
//! example, for a stepper motor, the stop command would ramp the speed down
//! before stopping the motor while emergency stop would stop stepping
//! immediately; in the later case, it is possible that the motor will spin
//! a couple of additional steps, so position accuracy is sacrificed.  This
//! is needed for safety reasons.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_EMERGENCY_STOP {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_EMERGENCY_STOP {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_EMERGENCY_STOP      0x32

//*****************************************************************************
//
//! Sets the references for three RC servo motors
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x0A CMD_SET_MOTOR_REFS
//!         M1_MB M1_LB
//!         M2_MB M2_LB
//!         M3_MB M3_LB
//!     {checksum}
//! \endcode
//!
//! \note \b Mx - motor x, \b MB - most significant byte, \b LB - least
//! significant byte.
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x04 CMD_SET_MOTOR_REFS {checksum}
//! \endcode
//
//*****************************************************************************
#define CMD_SET_MOTOR_REFS		0x40

//*****************************************************************************
//
//! Demands sensor data.
//!
//! <i>Command:</i>
//! \code
//!     TAG_CMD 0x04 CMD_GET_SENSOR_DATA {checksum}
//! \endcode
//!
//! <i>Response:</i>
//! \code
//!     TAG_STATUS 0x16 CMD_GET_SENSOR_DATA
//!          IMU_XACCL_MB IMU_XACCL_LB
//!          IMU_YACCL_MB IMU_YACCL_LB
//!          IMU_ZACCL_MB IMU_ZACCL_LB
//!          IMU_XGYRO_MB IMU_XGYRO_LB
//!          IMU_YGYRO_MB IMU_YGYRO_LB
//!          IMU_ZGYRO_MB IMU_ZGYRO_LB
//!          IMU_XMAGN_MB IMU_XMAGN_LB
//!          IMU_YMAGN_MB IMU_YMAGN_LB
//!          IMU_ZMAGN_MB IMU_ZMAGN_LB
//!     {checksum}
//! \endcode
//!
//! \note \b MB - most significant byte, \b LB - least
//! significant byte.
//
//*****************************************************************************
#define CMD_GET_SENSOR_DATA		0x41

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
//! \defgroup ui_common_api_params Parameters
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Specifies the version of the firmware on the motor drive.
//
//*****************************************************************************
#define PARAM_FIRMWARE_VERSION  0x00

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
//! \defgroup ui_common_api_rtdata Real-time data items
//! @{
//
//*****************************************************************************
#define DATA_IMU_SOUT	0x00	//!< IMU Supply out
#define DATA_IMU_XGYRO	0x01	//!< IMU X gyroscope output
#define DATA_IMU_YGYRO	0x02
#define DATA_IMU_ZGYRO	0x03

#define DATA_IMU_XACCL	0x04
#define DATA_IMU_YACCL	0x05
#define DATA_IMU_ZACCL	0x06

#define DATA_IMU_XMAGN	0x07
#define DATA_IMU_YMAGN	0x08
#define DATA_IMU_ZMAGN	0x09

#define DATA_IMU_TOUT	0x0A
#define DATA_IMU_AUXADC	0x0B

//*****************************************************************************
//
//! The number of real-time data items.
//
//*****************************************************************************
#define DATA_NUM_ITEMS          0x0C

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
//! \defgroup ui_common_api_stats Statuses
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! This is the device status when the device is stopped.
//
//*****************************************************************************
#define DEVICE_STATUS_STOP       0x00

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
