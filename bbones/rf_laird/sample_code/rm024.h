/*******************************************************************************
 *  rm024.h
 *
 *  Generic library for interfacing with the RM024 Wireless Serial device.
 *
 *  January 3, 2013
 *  Version 1.0.0
 *
 *  wireless.support@lairdtech.com
 *
 *  Copyright (c) 2012-2013, Laird Technologies, Inc - http://www.lairdtech.com
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Laird Technologies, Inc nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LAIRD TECHNOLOGIES, INC BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
#ifndef RM024_H_INCLUDED
#define RM024_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __unix__

#include <unistd.h>

typedef int HANDLE;

#elif defined _WIN32

#include <windows.h>

#else

#define EMBEDDED

#include "embedded.h"

#endif

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef unsigned char BYTE;

typedef enum
{
    OFF,
    DISABLE = 0,
    ON,
    ENABLE = 1
} STATE;

typedef enum
{
    MS,
    S,
    M,
    H
} TIMEBASE;

typedef enum
{
    ENTER_COMMAND_MODE,
    EXIT_COMMAND_MODE,
    ENTER_SLEEP,                            // (..,Mode,Res,Timer_H,Timer_L)
        SLEEP_W_INTERRUPT,
        SLEEP_W_TIMER,                      // (..,Res,Timer_H,Timer_L)
    SOFT_RESET,
    RESTORE_RESET,
    STATUS_REQUEST,
    CHECK_STATUS_REG,
    CHECK_FIRMWARE_STATUS,
    BIN_ANALYZER,                           // (..,Data,NumRuns)
    READ_TEMPERATURE,
    CHANGE_CHANNEL,                         // (..,Channel)
    SET_SERVER_CLIENT,                      // (..,Data)
        SET_TO_CLIENT,
        SET_TO_SERVER,
    SET_BROADCAST_MODE,                     // (..,Data)
    READ_IRAM,                              // (..,Location)
    WRITE_IRAM,                             // (..,Location,Value)
        IRAM_RANGE_REFRESH,                 // (..,Value)
        IRAM_RF_CHANNEL,
        IRAM_INTERFACE_TIMEOUT,
        IRAM_RF_PACKET_SIZE,
        IRAM_CTS_ON_HIGH,
        IRAM_CTS_ON_LOW,
        IRAM_CTS_OFF_HIGH,
        IRAM_CTS_OFF_LOW,
        IRAM_MAX_POWER,
        IRAM_DEST_ADDR_3,
        IRAM_DEST_ADDR_2,
        IRAM_DEST_ADDR_1,
        IRAM_DEST_ADDR_0,
        IRAM_SYSTEM_ID,
    WRITE_DEST_ADDR,                        // (..,LastThreeBytes)
    READ_DEST_ADDR,
    AUTO_DEST_CHANNEL,                      // (..,Data)
        AUTO_CHANNEL,
        AUTO_DESTINATION,
    READ_API_CONTROL,
    WRITE_API_CONTROL,                      // (..,APIControl)
        API_SEND_DATA_COMPLETE,
        API_TRANSMIT,
        API_RECIEVE,
    READ_ADC,                               // (..,Data)
    GET_LAST_RSSI,
    READ_DIGITAL_INPUT,
    WRITE_DIGITAL_OUT,                      // (..,Data)
        DIGITAL_IO_0,
        DIGITAL_IO_1,
    WRITE_PWM,                              // (..,Data)
    SET_POWER_CONTROL,                      // (..,Power)
        SET_POWER_FULL,
        SET_POWER_HALF,
        SET_POWER_QUARTER,
        SET_POWER_LOW,
    ANTENNA_SELECT,                         // (..,Port)
        INTEGRATED_ANTENNA,
        UFL_PORT_ANTENNA,
    DECRYPT_NEW_IMAGE,
    ERASE_FLASH,
    READ_FLASH,                             // (..,Start_H,Start_L,Length_H,Length_L)
    WRITE_FLASH,                            // (..,Start_H,Start_L,Length_H,Length_L,Data)
    SET_VENDOR_ID,                          // (..,Vid_H,Vid_L) WARNING: Cannot be undone once written!
    CHECK_VENDOR_ID,
    COMMAND_LENGTH
} COMMAND;

typedef enum
{
    EEPROM_BYTE_READ,                       // (..,Start,Length)
    EEPROM_BYTE_WRITE,                      // (..,Start,Length,Data)
        EEPROM_PRODUCT_ID,
        EEPROM_RANGE_REFRESH,
        EEPROM_CHANNEL_NUMBER,
        EEPROM_SERVER_CLIENT,
            EEPROM_SET_SERVER,
            EEPROM_SET_CLIENT,
        EEPROM_BAUD_RATE,
            EEPROM_CUSTOM_BAUD,
        EEPROM_BAUD_M,
        EEPROM_BAUD_E,
        EEPROM_CONTROL_0,
            EEPROM_SLEEP_INDICATOR,
            EEPROM_AUTO_SYSTEM_ID,
            EEPROM_CMDDATA_RCV_DISABLE,
            EEPROM_LEGACY_RSSI,
            EEPROM_SNIFF_REPORT,
            EEPROM_SNIFF_PERMIT,
        EEPROM_TRANSMIT_RETRIES,
        EEPROM_BROADCAST_ATTEMPTS,
        EEPROM_UTILITY_RETRIES,
        EEPROM_RF_PROFILE,
            EEPROM_RF_500_43,
            EEPROM_RF_280_79_FCC,
            EEPROM_RF_280_43_FCC,
            EEPROM_RF_280_43,
        EEPROM_CONTROL_1,
            EEPROM_AUTO_DEST_ON_BEACONS,
            EEPROM_DISABLE_HOP_FRAME,
            EEPROM_AUTO_DESTINATION,
            EEPROM_CLIENT_AUTO_CHANNEL,
            EEPROM_RTS_HANDSHAKING,
            EEPROM_FULL_DUPLEX,
            EEPROM_AUTO_CONFIG,
        EEPROM_CONTROL_2,
            EEPROM_DISCARD_FRAMING_ERRORS,
            EEPROM_HOP_PACKET_DELINEATION,
            EEPROM_OVERRIDE_485_TIMING,
            EEPROM_REMOTE_ANALOG_ENABLE,
            EEPROM_REMOTE_IO_MODE,
            EEPROM_RS485_DATA_ENABLE,
            EEPROM_NINE_BIT_MODE,
            EEPROM_9600_BOOT_OPTION,
        EEPROM_INTERFACE_TIMEOUT,
        EEPROM_ANTENNA_OVERRIDE,
        EEPROM_RF_PACKET_SIZE,
        EEPROM_CTS_ON,
        EEPROM_CTS_OFF,
        EEPROM_REMOTE_IO_CONTROL,
            EEPROM_USE_PAIRS,
            EEPROM_ALL_INPUTS,
            EEPROM_RXD_TXD_PAIR,
            EEPROM_RTS_CTS_PAIR,
            EEPROM_CMD_DATA_GIO2_PAIR,
            EEPROM_GIO7_GIO3_PAIR,
            EEPROM_GIO8_GIO1_PAIR,
            EEPROM_GIO4_GIO0_PAIR,
        EEPROM_SLEEP_CONTROL,
            EEPROM_CYCLIC_SLEEP,
        EEPROM_MAX_POWER,
            EEPROM_FULL_POWER,
            EEPROM_HALF_POWER,
            EEPROM_QUARTER_POWER,
            EEPROM_LOW_POWER,
        EEPROM_RSSI_THRESHOLD_HIGH,
        EEPROM_RSSI_THRESHOLD_LOW,
        EEPROM_RSSI_LAG,
        EEPROM_RSSI_CONTROL,
            EEPROM_PWM_OUTPORT_HIGH,
            EEPROM_PWM_OUTPORT_LOW,
            EEPROM_USE_AVERAGE_RSSI,
            EEPROM_INVERT_REPORT,
            EEPROM_UNINTENDED_REPORT,
            EEPROM_BROADCAST_REPORT,
            EEPROM_ADDRESSED_REPORT,
            EEPROM_BEACON_REPORT,
        EEPROM_BEACON_SKIP,
        EEPROM_DEST_MAC_ADDRESS,
        EEPROM_SYSTEM_ID,
        EEPROM_MAC_ADDRESS,
        EEPROM_PART_NUMBERS,
        EEPROM_USER_MEMORY,
        EEPROM_API_CONTROL,
            EEPROM_BROADCAST_MODE,
            EEPROM_INRANGE_HIGH_ON_WAKE,
            EEPROM_ANTENNA_SELECT,
            EEPROM_DISABLE_STATUS_BIN,
            EEPROM_UNICAST_ONLY,
            EEPROM_API_SEND_DATA_COMPLETE,
            EEPROM_API_TRANSMIT,
            EEPROM_API_RECEIVE,
        EEPROM_RANDOM_BACKOFF,
        EEPROM_PWM_INITIALIZE,
        EEPROM_REMOTE_IO_INIT_HIGH,
            EEPROM_RXD_INITIALIZE,
            EEPROM_RTS_INITIALIZE,
            EEPROM_CMD_DATA_INITIALIZE,
            EEPROM_GIO7_INITIALIZE,
            EEPROM_GIO8_INITIALIZE,
            EEPROM_GIO4_INITIALIZE,
        EEPROM_REMOTE_IO_INIT_LOW,
            EEPROM_TXD_INITIALIZE,
            EEPROM_CTS_INITIALIZE,
            EEPROM_GIO2_INITIALIZE,
            EEPROM_GIO3_INITIALIZE,
            EEPROM_GIO1_INITIALIZE,
            EEPROM_GIO0_INITIALIZE,
        EEPROM_SLEEP_TIME_HIGH,
        EEPROM_SLEEP_TIME_LOW,
        EEPROM_WAKE_COUNT,
        EEPROM_DATE_OF_BIRTH,
    EEPROM_LENGTH
} EEPROM;

typedef enum
{
    NO_ERRORS,
    COMMUNICATION_ERROR,
    UNRECOGNIZED_COMMAND,
    UNRECOGNIZED_RESPONSE,
    COMMAND_MODE_FAILED,
    INVALID_ARGUMENT,
    TIME_TOO_LARGE
} ERR;


/*
 *  Writes a command to an RM024 device connected to the given port
 *   and checks for the proper response. Assumes port is already connected.
 *
 *  INPUTS: device - the handle for the connected port
 *          command - a valid command keyword from the COMMAND list
 *          [...] - some keywords require additional parameters (BYTE*)
 *
 *  OUTPUTS: ERR - error code
 */
extern ERR WriteRM024(const HANDLE device, COMMAND command, ...);


/*
 *  Reads information from an RM024 device connected to the given port.
 *   Assumes port is already connected.
 *
 *  INPUTS: responseOut - pointer for returned data
 *          device - the handle for the connected port
 *          command - a valid command keyword from the COMMAND list
 *          [...] - some keywords require additional parameters (BYTE*)
 *
 *  OUTPUTS: ERR - error code
 */
extern ERR ReadRM024(BYTE *responseOut, const HANDLE device, COMMAND command, ...);


/*
 *  Writes a command to the RM024 EEPROM connected to the given port
 *   and checks for the proper response. Assumes port is already connected.
 *
 *  INPUTS: device - the handle for the connected port
 *          command - a valid command keyword from the COMMAND list
 *          [...] - some keywords require additional parameters (BYTE*)
 *
 *  OUTPUTS: ERR - error code
 */
extern ERR WriteRM024_EEPROM(const HANDLE device, EEPROM command, ...);


/*
 *  Reads information from the RM024 EEPROM connected to the given port.
 *   Assumes port is already connected.
 *
 *  INPUTS: responseOut - pointer for returned data
 *          device - the handle for the connected port
 *          command - a valid command keyword from the COMMAND list
 *          [...] - some keywords require additional parameters (BYTE*)
 *
 *  OUTPUTS: ERR - error code
 */
extern ERR ReadRM024_EEPROM(BYTE *responseOut, const HANDLE device, EEPROM command, ...);


/*
 *  Converts the given time to three bytes that can be understood by the RM024
 *   SLEEP_W_TIMER function.
 *
 *  INPUTS: time - amount of time to sleep, must be between
 *                  13 milliseconds and 18.2 hours
 *          base - a timebase for the given time. Options are MS, S, M, H
 *                  for milliseconds, seconds, minutes, hours
 *          sleepArgs - pointers for returning properly formatted
 *                                              arguments for the SLEEP_W_TIMER command
 *
 *  OUTPUTS: ERR - error code
 */
extern ERR FormatSleepTime(unsigned int time, TIMEBASE base, BYTE *sleepArgs);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RM024_H_INCLUDED */
