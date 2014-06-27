/*******************************************************************************
 *  rm024.c
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
#include "rm024.h"


/*****************************************************
 *  DECLARATIONS
 *****************************************************/
typedef struct
{
    unsigned int callSize;  // Number of bytes defined in call
    BYTE call[6];           // Holds specific hexcode for this command
    unsigned int respSize;  // Number of bytes defined in response
    BYTE response[4];       // Holds specific hexcode for the response from a device after a call
    unsigned int argSize;   // Number of bytes needed from a user to create a complete hexcode call
    unsigned int dataSize;  // Number of bytes expected after response (any data not known beforehand)
    BYTE *finalCall;        // Pointer to array that will hold the call after constructing the final form
    BYTE *finalResp;        // Pointer to array that will hold the response after constructing the final form
} ATCommand;

#ifndef EMBEDDED    /* Skip these if on embedded processors */
    BYTE* Read(const HANDLE device, int length);
    int Write(const HANDLE device, const BYTE *buffer, int length);
    void delay(double time);
#endif /* EMBEDDED */

void alignWrite(const HANDLE device, COMMAND command, ATCommand *ATCommandIn, BYTE *input);
void alignRead(COMMAND command, ATCommand *ATCommandIn, BYTE *input);
void alignWrite_EEPROM(const HANDLE device, EEPROM command, ATCommand *ATCommandIn, BYTE *input);
void alignRead_EEPROM(EEPROM command, ATCommand *ATCommandIn, BYTE *input);


/*
 * Command definitions array
 *
 * Every RM024 command is a hex code sent to the device, the device returns a hex response
 *
 * Ex. ENTER_COMMAND_MODE is a 6 byte command with no user defined arguments and a known 4 byte response,
 *      therefore, number of arguments = 0 and data size = 0. Conversely, SET_BROADCAST_MODE requires a
 *      1 byte argument and the last byte of the response is not predefined so arguments = 1 and data = 1
 *
 * In the interest of keeping memory usage low, some commands do not contain the hardcoded read or
 *      write hex code. These will be reconstructed in the align functions below.
 *
 */
const ATCommand RM024[] =
{
    [ENTER_COMMAND_MODE].callSize = 6, [ENTER_COMMAND_MODE].call = {0x41,0x54,0x2B,0x2B,0x2B,0x0D},
    [ENTER_COMMAND_MODE].respSize = 4, [ENTER_COMMAND_MODE].response = {0xCC,0x43,0x4F,0x4D},
    [ENTER_COMMAND_MODE].argSize = 0,  [ENTER_COMMAND_MODE].dataSize = 0,

    [EXIT_COMMAND_MODE].callSize = 5, [EXIT_COMMAND_MODE].call = {0xCC,0x41,0x54,0x4F,0x0D},
    [EXIT_COMMAND_MODE].respSize = 4, [EXIT_COMMAND_MODE].response = {0xCC,0x44,0x41,0x54},
    [EXIT_COMMAND_MODE].argSize = 0,  [EXIT_COMMAND_MODE].dataSize = 0,

    [ENTER_SLEEP].callSize = 2, [ENTER_SLEEP].call = {0xCC,0x86},
    [ENTER_SLEEP].respSize = 0, [ENTER_SLEEP].response = {0},
    [ENTER_SLEEP].argSize = 4,  [ENTER_SLEEP].dataSize = 0,

        [SLEEP_W_INTERRUPT].callSize = 3, [SLEEP_W_INTERRUPT].call = {0xCC,0x86,0x03},
        [SLEEP_W_INTERRUPT].respSize = 0, [SLEEP_W_INTERRUPT].response = {0},
        [SLEEP_W_INTERRUPT].argSize = 0,  [SLEEP_W_INTERRUPT].dataSize = 0,

        [SLEEP_W_TIMER].callSize = 3, [SLEEP_W_TIMER].call = {0xCC,0x86,0x02},
        [SLEEP_W_TIMER].respSize = 0, [SLEEP_W_TIMER].response = {0},
        [SLEEP_W_TIMER].argSize = 3,  [SLEEP_W_TIMER].dataSize = 0,

    [SOFT_RESET].callSize = 2, [SOFT_RESET].call = {0xCC,0xFF},
    [SOFT_RESET].respSize = 0, [SOFT_RESET].response = {0},
    [SOFT_RESET].argSize = 0,  [SOFT_RESET].dataSize = 0,

    [RESTORE_RESET].callSize = 3, [RESTORE_RESET].call = {0xCC,0xFF,0xDF},
    [RESTORE_RESET].respSize = 0, [RESTORE_RESET].response = {0},
    [RESTORE_RESET].argSize = 0,  [RESTORE_RESET].dataSize = 0,

    [STATUS_REQUEST].callSize = 3, [STATUS_REQUEST].call = {0xCC,0x00,0x00},
    [STATUS_REQUEST].respSize = 1, [STATUS_REQUEST].response = {0xCC},
    [STATUS_REQUEST].argSize = 0,  [STATUS_REQUEST].dataSize = 2,

    [CHECK_STATUS_REG].callSize = 3, [CHECK_STATUS_REG].call = {0xCC,0x00,0x01},
    [CHECK_STATUS_REG].respSize = 1, [CHECK_STATUS_REG].response = {0xCC},
    [CHECK_STATUS_REG].argSize = 0,  [CHECK_STATUS_REG].dataSize = 4,

    [CHECK_FIRMWARE_STATUS].callSize = 3, [CHECK_FIRMWARE_STATUS].call = {0xCC,0x00,0x02},
    [CHECK_FIRMWARE_STATUS].respSize = 2, [CHECK_FIRMWARE_STATUS].response = {0xCC,0x02},
    [CHECK_FIRMWARE_STATUS].argSize = 0,  [CHECK_FIRMWARE_STATUS].dataSize = 2,

    [BIN_ANALYZER].callSize = 2, [BIN_ANALYZER].call = {0xCC,0x8F},
    [BIN_ANALYZER].respSize = 1, [BIN_ANALYZER].response = {0xCC},
    [BIN_ANALYZER].argSize = 2,  [BIN_ANALYZER].dataSize = 1,

    [READ_TEMPERATURE].callSize = 2, [READ_TEMPERATURE].call = {0xCC,0xA4},
    [READ_TEMPERATURE].respSize = 1, [READ_TEMPERATURE].response = {0xCC},
    [READ_TEMPERATURE].argSize = 0,  [READ_TEMPERATURE].dataSize = 1,

    [CHANGE_CHANNEL].callSize = 2, [CHANGE_CHANNEL].call = {0xCC,0x02},
    [CHANGE_CHANNEL].respSize = 1, [CHANGE_CHANNEL].response = {0xCC},
    [CHANGE_CHANNEL].argSize = 1,  [CHANGE_CHANNEL].dataSize = 1,

    [SET_SERVER_CLIENT].callSize = 2, [SET_SERVER_CLIENT].call = {0xCC,0x03},
    [SET_SERVER_CLIENT].respSize = 1, [SET_SERVER_CLIENT].response = {0xCC},
    [SET_SERVER_CLIENT].argSize = 1,  [SET_SERVER_CLIENT].dataSize = 2,

        [SET_TO_CLIENT].callSize = 3, [SET_TO_CLIENT].call = {0xCC,0x03,0x03},
        [SET_TO_CLIENT].respSize = 1, [SET_TO_CLIENT].response = {0xCC},
        [SET_TO_CLIENT].argSize = 0,  [SET_TO_CLIENT].dataSize = 2,

        [SET_TO_SERVER].callSize = 3, [SET_TO_SERVER].call = {0xCC,0x03,0x02},
        [SET_TO_SERVER].respSize = 1, [SET_TO_SERVER].response = {0xCC},
        [SET_TO_SERVER].argSize = 0,  [SET_TO_SERVER].dataSize = 2,

    [SET_BROADCAST_MODE].callSize = 2, [SET_BROADCAST_MODE].call = {0xCC,0x08},
    [SET_BROADCAST_MODE].respSize = 1, [SET_BROADCAST_MODE].response = {0xCC},
    [SET_BROADCAST_MODE].argSize = 1,  [SET_BROADCAST_MODE].dataSize = 1,

    [READ_IRAM].callSize = 2, [READ_IRAM].call = {0xCC,0x0A},
    [READ_IRAM].respSize = 1, [READ_IRAM].response = {0xCC},
    [READ_IRAM].argSize = 1,  [READ_IRAM].dataSize = 1,

    [WRITE_IRAM].callSize = 2, [WRITE_IRAM].call = {0xCC,0x0B},
    [WRITE_IRAM].respSize = 1, [WRITE_IRAM].response = {0xCC},
    [WRITE_IRAM].argSize = 2,  [WRITE_IRAM].dataSize = 2,

        [IRAM_RANGE_REFRESH].callSize = 1, [IRAM_RANGE_REFRESH].call = {0x3D},
        [IRAM_RANGE_REFRESH].respSize = 0, [IRAM_RANGE_REFRESH].response = {0},
        [IRAM_RANGE_REFRESH].argSize = 1,  [IRAM_RANGE_REFRESH].dataSize = 0,

        [IRAM_RF_CHANNEL].callSize = 1, [IRAM_RF_CHANNEL].call = {0x41},
        [IRAM_RF_CHANNEL].respSize = 0, [IRAM_RF_CHANNEL].response = {0},
        [IRAM_RF_CHANNEL].argSize = 1,  [IRAM_RF_CHANNEL].dataSize = 0,

        [IRAM_INTERFACE_TIMEOUT].callSize = 1, [IRAM_INTERFACE_TIMEOUT].call = {0x58},
        [IRAM_INTERFACE_TIMEOUT].respSize = 0, [IRAM_INTERFACE_TIMEOUT].response = {0},
        [IRAM_INTERFACE_TIMEOUT].argSize = 1,  [IRAM_INTERFACE_TIMEOUT].dataSize = 0,

        [IRAM_RF_PACKET_SIZE].callSize = 1, [IRAM_RF_PACKET_SIZE].call = {0x5A},
        [IRAM_RF_PACKET_SIZE].respSize = 0, [IRAM_RF_PACKET_SIZE].response = {0},
        [IRAM_RF_PACKET_SIZE].argSize = 1,  [IRAM_RF_PACKET_SIZE].dataSize = 0,

        [IRAM_CTS_ON_HIGH].callSize = 1, [IRAM_CTS_ON_HIGH].call = {0x5C},
        [IRAM_CTS_ON_HIGH].respSize = 0, [IRAM_CTS_ON_HIGH].response = {0},
        [IRAM_CTS_ON_HIGH].argSize = 1,  [IRAM_CTS_ON_HIGH].dataSize = 0,

        [IRAM_CTS_ON_LOW].callSize = 1, [IRAM_CTS_ON_LOW].call = {0x5D},
        [IRAM_CTS_ON_LOW].respSize = 0, [IRAM_CTS_ON_LOW].response = {0},
        [IRAM_CTS_ON_LOW].argSize = 1,  [IRAM_CTS_ON_LOW].dataSize = 0,

        [IRAM_CTS_OFF_HIGH].callSize = 1, [IRAM_CTS_OFF_HIGH].call = {0x5E},
        [IRAM_CTS_OFF_HIGH].respSize = 0, [IRAM_CTS_OFF_HIGH].response = {0},
        [IRAM_CTS_OFF_HIGH].argSize = 1,  [IRAM_CTS_OFF_HIGH].dataSize = 0,

        [IRAM_CTS_OFF_LOW].callSize = 1, [IRAM_CTS_OFF_LOW].call = {0x5F},
        [IRAM_CTS_OFF_LOW].respSize = 0, [IRAM_CTS_OFF_LOW].response = {0},
        [IRAM_CTS_OFF_LOW].argSize = 1,  [IRAM_CTS_OFF_LOW].dataSize = 0,

        [IRAM_MAX_POWER].callSize = 1, [IRAM_MAX_POWER].call = {0x63},
        [IRAM_MAX_POWER].respSize = 0, [IRAM_MAX_POWER].response = {0},
        [IRAM_MAX_POWER].argSize = 1,  [IRAM_MAX_POWER].dataSize = 0,

        [IRAM_DEST_ADDR_3].callSize = 1, [IRAM_DEST_ADDR_3].call = {0x72},
        [IRAM_DEST_ADDR_3].respSize = 0, [IRAM_DEST_ADDR_3].response = {0},
        [IRAM_DEST_ADDR_3].argSize = 1,  [IRAM_DEST_ADDR_3].dataSize = 0,

        [IRAM_DEST_ADDR_2].callSize = 1, [IRAM_DEST_ADDR_2].call = {0x73},
        [IRAM_DEST_ADDR_2].respSize = 0, [IRAM_DEST_ADDR_2].response = {0},
        [IRAM_DEST_ADDR_2].argSize = 1,  [IRAM_DEST_ADDR_2].dataSize = 0,

        [IRAM_DEST_ADDR_1].callSize = 1, [IRAM_DEST_ADDR_1].call = {0x74},
        [IRAM_DEST_ADDR_1].respSize = 0, [IRAM_DEST_ADDR_1].response = {0},
        [IRAM_DEST_ADDR_1].argSize = 1,  [IRAM_DEST_ADDR_1].dataSize = 0,

        [IRAM_DEST_ADDR_0].callSize = 1, [IRAM_DEST_ADDR_0].call = {0x75},
        [IRAM_DEST_ADDR_0].respSize = 0, [IRAM_DEST_ADDR_0].response = {0},
        [IRAM_DEST_ADDR_0].argSize = 1,  [IRAM_DEST_ADDR_0].dataSize = 0,

        [IRAM_SYSTEM_ID].callSize = 1, [IRAM_SYSTEM_ID].call = {0x76},
        [IRAM_SYSTEM_ID].respSize = 0, [IRAM_SYSTEM_ID].response = {0},
        [IRAM_SYSTEM_ID].argSize = 1,  [IRAM_SYSTEM_ID].dataSize = 0,

    [WRITE_DEST_ADDR].callSize = 2, [WRITE_DEST_ADDR].call = {0xCC,0x10},
    [WRITE_DEST_ADDR].respSize = 1, [WRITE_DEST_ADDR].response = {0xCC},
    [WRITE_DEST_ADDR].argSize = 3,  [WRITE_DEST_ADDR].dataSize = 3,

    [READ_DEST_ADDR].callSize = 2, [READ_DEST_ADDR].call = {0xCC,0x11},
    [READ_DEST_ADDR].respSize = 1, [READ_DEST_ADDR].response = {0xCC},
    [READ_DEST_ADDR].argSize = 0,  [READ_DEST_ADDR].dataSize = 3,

    [AUTO_DEST_CHANNEL].callSize = 2, [AUTO_DEST_CHANNEL].call = {0xCC,0x15},
    [AUTO_DEST_CHANNEL].respSize = 1, [AUTO_DEST_CHANNEL].response = {0xCC},
    [AUTO_DEST_CHANNEL].argSize = 1,  [AUTO_DEST_CHANNEL].dataSize = 1,

        [AUTO_CHANNEL].callSize = 3, [AUTO_CHANNEL].call = {0xCC,0x15,0x22},
        [AUTO_CHANNEL].respSize = 1, [AUTO_CHANNEL].response = {0xCC},
        [AUTO_CHANNEL].argSize = 1,  [AUTO_CHANNEL].dataSize = 1,

        [AUTO_DESTINATION].callSize = 3, [AUTO_DESTINATION].call = {0xCC,0x15,0x11},
        [AUTO_DESTINATION].respSize = 1, [AUTO_DESTINATION].response = {0xCC},
        [AUTO_DESTINATION].argSize = 1,  [AUTO_DESTINATION].dataSize = 1,

    [READ_API_CONTROL].callSize = 2, [READ_API_CONTROL].call = {0xCC,0x16},
    [READ_API_CONTROL].respSize = 1, [READ_API_CONTROL].response = {0xCC},
    [READ_API_CONTROL].argSize = 0,  [READ_API_CONTROL].dataSize = 1,

    [WRITE_API_CONTROL].callSize = 2, [WRITE_API_CONTROL].call = {0xCC,0x17},
    [WRITE_API_CONTROL].respSize = 1, [WRITE_API_CONTROL].response = {0xCC},
    [WRITE_API_CONTROL].argSize = 1,  [WRITE_API_CONTROL].dataSize = 1,

        [API_SEND_DATA_COMPLETE].callSize = 1, [API_SEND_DATA_COMPLETE].call = {0x04},
        [API_SEND_DATA_COMPLETE].respSize = 1, [API_SEND_DATA_COMPLETE].response = {0xCC},
        [API_SEND_DATA_COMPLETE].argSize = 1,  [API_SEND_DATA_COMPLETE].dataSize = 1,

        [API_TRANSMIT].callSize = 1, [API_TRANSMIT].call = {0x02},
        [API_TRANSMIT].respSize = 1, [API_TRANSMIT].response = {0xCC},
        [API_TRANSMIT].argSize = 1,  [API_TRANSMIT].dataSize = 1,

        [API_RECIEVE].callSize = 1, [API_RECIEVE].call = {0x01},
        [API_RECIEVE].respSize = 1, [API_RECIEVE].response = {0xCC},
        [API_RECIEVE].argSize = 1,  [API_RECIEVE].dataSize = 1,

    [READ_ADC].callSize = 2, [READ_ADC].call = {0xCC,0x21},
    [READ_ADC].respSize = 1, [READ_ADC].response = {0xCC},
    [READ_ADC].argSize = 1,  [READ_ADC].dataSize = 2,

    [GET_LAST_RSSI].callSize = 2, [GET_LAST_RSSI].call = {0xCC,0x22},
    [GET_LAST_RSSI].respSize = 1, [GET_LAST_RSSI].response = {0xCC},
    [GET_LAST_RSSI].argSize = 0,  [GET_LAST_RSSI].dataSize = 1,

    [READ_DIGITAL_INPUT].callSize = 2, [READ_DIGITAL_INPUT].call = {0xCC,0x20},
    [READ_DIGITAL_INPUT].respSize = 1, [READ_DIGITAL_INPUT].response = {0xCC},
    [READ_DIGITAL_INPUT].argSize = 0,  [READ_DIGITAL_INPUT].dataSize = 1,

    [WRITE_DIGITAL_OUT].callSize = 2, [WRITE_DIGITAL_OUT].call = {0xCC,0x23},
    [WRITE_DIGITAL_OUT].respSize = 1, [WRITE_DIGITAL_OUT].response = {0xCC},
    [WRITE_DIGITAL_OUT].argSize = 1,  [WRITE_DIGITAL_OUT].dataSize = 1,

        [DIGITAL_IO_0].callSize = 1, [DIGITAL_IO_0].call = {0x02},
        [DIGITAL_IO_0].respSize = 1, [DIGITAL_IO_0].response = {0xCC},
        [DIGITAL_IO_0].argSize = 1,  [DIGITAL_IO_0].dataSize = 1,

        [DIGITAL_IO_1].callSize = 1, [DIGITAL_IO_1].call = {0x01},
        [DIGITAL_IO_1].respSize = 1, [DIGITAL_IO_1].response = {0xCC},
        [DIGITAL_IO_1].argSize = 1,  [DIGITAL_IO_1].dataSize = 1,

    [WRITE_PWM].callSize = 2, [WRITE_PWM].call = {0xCC,0x24},
    [WRITE_PWM].respSize = 1, [WRITE_PWM].response = {0xCC},
    [WRITE_PWM].argSize = 1,  [WRITE_PWM].dataSize = 1,

    [SET_POWER_CONTROL].callSize = 2, [SET_POWER_CONTROL].call = {0xCC,0x25},
    [SET_POWER_CONTROL].respSize = 1, [SET_POWER_CONTROL].response = {0xCC},
    [SET_POWER_CONTROL].argSize = 1,  [SET_POWER_CONTROL].dataSize = 1,

        [SET_POWER_FULL].callSize = 3, [SET_POWER_FULL].call = {0xCC,0x25,0x00},
        [SET_POWER_FULL].respSize = 2, [SET_POWER_FULL].response = {0xCC,0x00},
        [SET_POWER_FULL].argSize = 0,  [SET_POWER_FULL].dataSize = 0,

        [SET_POWER_HALF].callSize = 3, [SET_POWER_HALF].call = {0xCC,0x25,0x01},
        [SET_POWER_HALF].respSize = 2, [SET_POWER_HALF].response = {0xCC,0x01},
        [SET_POWER_HALF].argSize = 0,  [SET_POWER_HALF].dataSize = 0,

        [SET_POWER_QUARTER].callSize = 3, [SET_POWER_QUARTER].call = {0xCC,0x25,0x02},
        [SET_POWER_QUARTER].respSize = 2, [SET_POWER_QUARTER].response = {0xCC,0x02},
        [SET_POWER_QUARTER].argSize = 0,  [SET_POWER_QUARTER].dataSize = 0,

        [SET_POWER_LOW].callSize = 3, [SET_POWER_LOW].call = {0xCC,0x25,0x03},
        [SET_POWER_LOW].respSize = 2, [SET_POWER_LOW].response = {0xCC,0x03},
        [SET_POWER_LOW].argSize = 0,  [SET_POWER_LOW].dataSize = 0,

    [ANTENNA_SELECT].callSize = 2, [ANTENNA_SELECT].call = {0xCC,0x26},
    [ANTENNA_SELECT].respSize = 2, [ANTENNA_SELECT].response = {0xCC,0x26},
    [ANTENNA_SELECT].argSize = 1,  [ANTENNA_SELECT].dataSize = 1,

        [INTEGRATED_ANTENNA].callSize = 3, [INTEGRATED_ANTENNA].call = {0xCC,0x26,0x00},
        [INTEGRATED_ANTENNA].respSize = 3, [INTEGRATED_ANTENNA].response = {0xCC,0x26,0x00},
        [INTEGRATED_ANTENNA].argSize = 0,  [INTEGRATED_ANTENNA].dataSize = 0,

        [UFL_PORT_ANTENNA].callSize = 3, [UFL_PORT_ANTENNA].call = {0xCC,0x26,0x01},
        [UFL_PORT_ANTENNA].respSize = 3, [UFL_PORT_ANTENNA].response = {0xCC,0x26,0x01},
        [UFL_PORT_ANTENNA].argSize = 0,  [UFL_PORT_ANTENNA].dataSize = 0,

    [DECRYPT_NEW_IMAGE].callSize = 2, [DECRYPT_NEW_IMAGE].call = {0xCC,0xC5},
    [DECRYPT_NEW_IMAGE].respSize = 3, [DECRYPT_NEW_IMAGE].response = {0xCC,0xC5,0x00},
    [DECRYPT_NEW_IMAGE].argSize = 0,  [DECRYPT_NEW_IMAGE].dataSize = 0,

    [ERASE_FLASH].callSize = 2, [ERASE_FLASH].call = {0xCC,0xC6},
    [ERASE_FLASH].respSize = 2, [ERASE_FLASH].response = {0xCC,0xC6},
    [ERASE_FLASH].argSize = 0,  [ERASE_FLASH].dataSize = 0,

    [READ_FLASH].callSize = 2, [READ_FLASH].call = {0xCC,0xC9},
    [READ_FLASH].respSize = 2, [READ_FLASH].response = {0xCC,0xC9},
    [READ_FLASH].argSize = 4,  [READ_FLASH].dataSize = 5,

    [WRITE_FLASH].callSize = 2, [WRITE_FLASH].call = {0xCC,0xC4},
    [WRITE_FLASH].respSize = 3, [WRITE_FLASH].response = {0xCC,0xC4,0x00},
    [WRITE_FLASH].argSize = 4,  [WRITE_FLASH].dataSize = 2,

    [SET_VENDOR_ID].callSize = 3, [SET_VENDOR_ID].call = {0xCC,0xF2,0x06},
    [SET_VENDOR_ID].respSize = 3, [SET_VENDOR_ID].response = {0xCC,0xF2,0x06},
    [SET_VENDOR_ID].argSize = 2,  [SET_VENDOR_ID].dataSize = 2,

    [CHECK_VENDOR_ID].callSize = 5, [CHECK_VENDOR_ID].call = {0xCC,0xF2,0x06,0xFF,0xFF},
    [CHECK_VENDOR_ID].respSize = 3, [CHECK_VENDOR_ID].response = {0xCC,0xF2,0x06},
    [CHECK_VENDOR_ID].argSize = 0,  [CHECK_VENDOR_ID].dataSize = 2,
};

/*
 *  EEPROM command definitions array
 *
 *  In the interest of keeping memory usage low, commands below BYTE_READ/BYTE_WRITE do
 *      not contain the hardcoded read or write hex code. These will be reconstructed
 *      in the align functions below.
 *
 */
const ATCommand EEPROM024[] =
{
    [EEPROM_BYTE_READ].callSize = 2, [EEPROM_BYTE_READ].call = {0xCC,0xC0},
    [EEPROM_BYTE_READ].respSize = 1, [EEPROM_BYTE_READ].response = {0xCC},
    [EEPROM_BYTE_READ].argSize = 2,  [EEPROM_BYTE_READ].dataSize = 2,

    [EEPROM_BYTE_WRITE].callSize = 2, [EEPROM_BYTE_WRITE].call = {0xCC,0xC1},
    [EEPROM_BYTE_WRITE].respSize = 2, [EEPROM_BYTE_WRITE].response = {0},
    [EEPROM_BYTE_WRITE].argSize = 2,  [EEPROM_BYTE_WRITE].dataSize = 1,

        [EEPROM_PRODUCT_ID].callSize = 2, [EEPROM_PRODUCT_ID].call = {0x00,0x17},
        [EEPROM_PRODUCT_ID].respSize = 0, [EEPROM_PRODUCT_ID].response = {0},
        [EEPROM_PRODUCT_ID].argSize = 35,  [EEPROM_PRODUCT_ID].dataSize = 35,

        [EEPROM_RANGE_REFRESH].callSize = 2, [EEPROM_RANGE_REFRESH].call = {0x3D,0x01},
        [EEPROM_RANGE_REFRESH].respSize = 0, [EEPROM_RANGE_REFRESH].response = {0},
        [EEPROM_RANGE_REFRESH].argSize = 1,  [EEPROM_RANGE_REFRESH].dataSize = 0,

        [EEPROM_CHANNEL_NUMBER].callSize = 2, [EEPROM_CHANNEL_NUMBER].call = {0x40,0x01},
        [EEPROM_CHANNEL_NUMBER].respSize = 0, [EEPROM_CHANNEL_NUMBER].response = {0},
        [EEPROM_CHANNEL_NUMBER].argSize = 1,  [EEPROM_CHANNEL_NUMBER].dataSize = 0,

        [EEPROM_SERVER_CLIENT].callSize = 2, [EEPROM_SERVER_CLIENT].call = {0x41,0x01},
        [EEPROM_SERVER_CLIENT].respSize = 0, [EEPROM_SERVER_CLIENT].response = {0},
        [EEPROM_SERVER_CLIENT].argSize = 1,  [EEPROM_SERVER_CLIENT].dataSize = 0,

            [EEPROM_SET_SERVER].callSize = 3, [EEPROM_SET_SERVER].call = {0x41,0x01,0x01},
            [EEPROM_SET_SERVER].respSize = 0, [EEPROM_SET_SERVER].response = {0},
            [EEPROM_SET_SERVER].argSize = 0,  [EEPROM_SET_SERVER].dataSize = 0,

            [EEPROM_SET_CLIENT].callSize = 3, [EEPROM_SET_CLIENT].call = {0x41,0x01,0x02},
            [EEPROM_SET_CLIENT].respSize = 0, [EEPROM_SET_CLIENT].response = {0},
            [EEPROM_SET_CLIENT].argSize = 0,  [EEPROM_SET_CLIENT].dataSize = 0,

        [EEPROM_BAUD_RATE].callSize = 2, [EEPROM_BAUD_RATE].call = {0x42,0x01},
        [EEPROM_BAUD_RATE].respSize = 0, [EEPROM_BAUD_RATE].response = {0},
        [EEPROM_BAUD_RATE].argSize = 1,  [EEPROM_BAUD_RATE].dataSize = 0,

            [EEPROM_CUSTOM_BAUD].callSize = 3, [EEPROM_CUSTOM_BAUD].call = {0x42,0x01,0xE3},
            [EEPROM_CUSTOM_BAUD].respSize = 0, [EEPROM_CUSTOM_BAUD].response = {0},
            [EEPROM_CUSTOM_BAUD].argSize = 0,  [EEPROM_CUSTOM_BAUD].dataSize = 0,

        [EEPROM_BAUD_M].callSize = 2, [EEPROM_BAUD_M].call = {0x43,0x01},
        [EEPROM_BAUD_M].respSize = 0, [EEPROM_BAUD_M].response = {0},
        [EEPROM_BAUD_M].argSize = 1,  [EEPROM_BAUD_M].dataSize = 0,

        [EEPROM_BAUD_E].callSize = 2, [EEPROM_BAUD_E].call = {0x44,0x01},
        [EEPROM_BAUD_E].respSize = 0, [EEPROM_BAUD_E].response = {0},
        [EEPROM_BAUD_E].argSize = 1,  [EEPROM_BAUD_E].dataSize = 0,

        [EEPROM_CONTROL_0].callSize = 2, [EEPROM_CONTROL_0].call = {0x45,0x01},
        [EEPROM_CONTROL_0].respSize = 0, [EEPROM_CONTROL_0].response = {0},
        [EEPROM_CONTROL_0].argSize = 1,  [EEPROM_CONTROL_0].dataSize = 0,

            [EEPROM_SLEEP_INDICATOR].callSize = 3, [EEPROM_SLEEP_INDICATOR].call = {0x45,0x01,0x40},
            [EEPROM_SLEEP_INDICATOR].respSize = 0, [EEPROM_SLEEP_INDICATOR].response = {0},
            [EEPROM_SLEEP_INDICATOR].argSize = 1,  [EEPROM_SLEEP_INDICATOR].dataSize = 0,

            [EEPROM_AUTO_SYSTEM_ID].callSize = 3, [EEPROM_AUTO_SYSTEM_ID].call = {0x45,0x01,0x10},
            [EEPROM_AUTO_SYSTEM_ID].respSize = 0, [EEPROM_AUTO_SYSTEM_ID].response = {0},
            [EEPROM_AUTO_SYSTEM_ID].argSize = 1,  [EEPROM_AUTO_SYSTEM_ID].dataSize = 0,

            [EEPROM_CMDDATA_RCV_DISABLE].callSize = 3, [EEPROM_CMDDATA_RCV_DISABLE].call = {0x45,0x01,0x08},
            [EEPROM_CMDDATA_RCV_DISABLE].respSize = 0, [EEPROM_CMDDATA_RCV_DISABLE].response = {0},
            [EEPROM_CMDDATA_RCV_DISABLE].argSize = 1,  [EEPROM_CMDDATA_RCV_DISABLE].dataSize = 0,

            [EEPROM_LEGACY_RSSI].callSize = 3, [EEPROM_LEGACY_RSSI].call = {0x45,0x01,0x04},
            [EEPROM_LEGACY_RSSI].respSize = 0, [EEPROM_LEGACY_RSSI].response = {0},
            [EEPROM_LEGACY_RSSI].argSize = 1,  [EEPROM_LEGACY_RSSI].dataSize = 0,

            [EEPROM_SNIFF_REPORT].callSize = 3, [EEPROM_SNIFF_REPORT].call = {0x45,0x01,0x02},
            [EEPROM_SNIFF_REPORT].respSize = 0, [EEPROM_SNIFF_REPORT].response = {0},
            [EEPROM_SNIFF_REPORT].argSize = 1,  [EEPROM_SNIFF_REPORT].dataSize = 0,

            [EEPROM_SNIFF_PERMIT].callSize = 3, [EEPROM_SNIFF_PERMIT].call = {0x45,0x01,0x01},
            [EEPROM_SNIFF_PERMIT].respSize = 0, [EEPROM_SNIFF_PERMIT].response = {0},
            [EEPROM_SNIFF_PERMIT].argSize = 1,  [EEPROM_SNIFF_PERMIT].dataSize = 0,

        [EEPROM_TRANSMIT_RETRIES].callSize = 2, [EEPROM_TRANSMIT_RETRIES].call = {0x4C,0x01},
        [EEPROM_TRANSMIT_RETRIES].respSize = 0, [EEPROM_TRANSMIT_RETRIES].response = {0},
        [EEPROM_TRANSMIT_RETRIES].argSize = 1,  [EEPROM_TRANSMIT_RETRIES].dataSize = 0,

        [EEPROM_BROADCAST_ATTEMPTS].callSize = 2, [EEPROM_BROADCAST_ATTEMPTS].call = {0x4D,0x01},
        [EEPROM_BROADCAST_ATTEMPTS].respSize = 0, [EEPROM_BROADCAST_ATTEMPTS].response = {0},
        [EEPROM_BROADCAST_ATTEMPTS].argSize = 1,  [EEPROM_BROADCAST_ATTEMPTS].dataSize = 0,

        [EEPROM_UTILITY_RETRIES].callSize = 2, [EEPROM_UTILITY_RETRIES].call = {0x4E,0x01},
        [EEPROM_UTILITY_RETRIES].respSize = 0, [EEPROM_UTILITY_RETRIES].response = {0},
        [EEPROM_UTILITY_RETRIES].argSize = 1,  [EEPROM_UTILITY_RETRIES].dataSize = 0,

        [EEPROM_RF_PROFILE].callSize = 2, [EEPROM_RF_PROFILE].call = {0x54,0x01},
        [EEPROM_RF_PROFILE].respSize = 0, [EEPROM_RF_PROFILE].response = {0},
        [EEPROM_RF_PROFILE].argSize = 1,  [EEPROM_RF_PROFILE].dataSize = 0,

            [EEPROM_RF_500_43].callSize = 3, [EEPROM_RF_500_43].call = {0x54,0x01,0x00},
            [EEPROM_RF_500_43].respSize = 0, [EEPROM_RF_500_43].response = {0},
            [EEPROM_RF_500_43].argSize = 0,  [EEPROM_RF_500_43].dataSize = 0,

            [EEPROM_RF_280_79_FCC].callSize = 3, [EEPROM_RF_280_79_FCC].call = {0x54,0x01,0x01},
            [EEPROM_RF_280_79_FCC].respSize = 0, [EEPROM_RF_280_79_FCC].response = {0},
            [EEPROM_RF_280_79_FCC].argSize = 0,  [EEPROM_RF_280_79_FCC].dataSize = 0,

            [EEPROM_RF_280_43_FCC].callSize = 3, [EEPROM_RF_280_43_FCC].call = {0x54,0x01,0x02},
            [EEPROM_RF_280_43_FCC].respSize = 0, [EEPROM_RF_280_43_FCC].response = {0},
            [EEPROM_RF_280_43_FCC].argSize = 0,  [EEPROM_RF_280_43_FCC].dataSize = 0,

            [EEPROM_RF_280_43].callSize = 3, [EEPROM_RF_280_43].call = {0x54,0x01,0x03},
            [EEPROM_RF_280_43].respSize = 0, [EEPROM_RF_280_43].response = {0},
            [EEPROM_RF_280_43].argSize = 0,  [EEPROM_RF_280_43].dataSize = 0,

        [EEPROM_CONTROL_1].callSize = 2, [EEPROM_CONTROL_1].call = {0x56,0x01},
        [EEPROM_CONTROL_1].respSize = 0, [EEPROM_CONTROL_1].response = {0},
        [EEPROM_CONTROL_1].argSize = 1,  [EEPROM_CONTROL_1].dataSize = 0,

            [EEPROM_AUTO_DEST_ON_BEACONS].callSize = 3, [EEPROM_AUTO_DEST_ON_BEACONS].call = {0x56,0x01,0x80},
            [EEPROM_AUTO_DEST_ON_BEACONS].respSize = 0, [EEPROM_AUTO_DEST_ON_BEACONS].response = {0},
            [EEPROM_AUTO_DEST_ON_BEACONS].argSize = 1,  [EEPROM_AUTO_DEST_ON_BEACONS].dataSize = 0,

            [EEPROM_DISABLE_HOP_FRAME].callSize = 3, [EEPROM_DISABLE_HOP_FRAME].call = {0x56,0x01,0x40},
            [EEPROM_DISABLE_HOP_FRAME].respSize = 0, [EEPROM_DISABLE_HOP_FRAME].response = {0},
            [EEPROM_DISABLE_HOP_FRAME].argSize = 1,  [EEPROM_DISABLE_HOP_FRAME].dataSize = 0,

            [EEPROM_AUTO_DESTINATION].callSize = 3, [EEPROM_AUTO_DESTINATION].call = {0x56,0x01,0x10},
            [EEPROM_AUTO_DESTINATION].respSize = 0, [EEPROM_AUTO_DESTINATION].response = {0},
            [EEPROM_AUTO_DESTINATION].argSize = 1,  [EEPROM_AUTO_DESTINATION].dataSize = 0,

            [EEPROM_CLIENT_AUTO_CHANNEL].callSize = 3, [EEPROM_CLIENT_AUTO_CHANNEL].call = {0x56,0x01,0x08},
            [EEPROM_CLIENT_AUTO_CHANNEL].respSize = 0, [EEPROM_CLIENT_AUTO_CHANNEL].response = {0},
            [EEPROM_CLIENT_AUTO_CHANNEL].argSize = 1,  [EEPROM_CLIENT_AUTO_CHANNEL].dataSize = 0,

            [EEPROM_RTS_HANDSHAKING].callSize = 3, [EEPROM_RTS_HANDSHAKING].call = {0x56,0x01,0x04},
            [EEPROM_RTS_HANDSHAKING].respSize = 0, [EEPROM_RTS_HANDSHAKING].response = {0},
            [EEPROM_RTS_HANDSHAKING].argSize = 1,  [EEPROM_RTS_HANDSHAKING].dataSize = 0,

            [EEPROM_FULL_DUPLEX].callSize = 3, [EEPROM_FULL_DUPLEX].call = {0x56,0x01,0x02},
            [EEPROM_FULL_DUPLEX].respSize = 0, [EEPROM_FULL_DUPLEX].response = {0},
            [EEPROM_FULL_DUPLEX].argSize = 1,  [EEPROM_FULL_DUPLEX].dataSize = 0,

            [EEPROM_AUTO_CONFIG].callSize = 3, [EEPROM_AUTO_CONFIG].call = {0x56,0x01,0x01},
            [EEPROM_AUTO_CONFIG].respSize = 0, [EEPROM_AUTO_CONFIG].response = {0},
            [EEPROM_AUTO_CONFIG].argSize = 1,  [EEPROM_AUTO_CONFIG].dataSize = 0,

        [EEPROM_CONTROL_2].callSize = 2, [EEPROM_CONTROL_2].call = {0x57,0x01},
        [EEPROM_CONTROL_2].respSize = 0, [EEPROM_CONTROL_2].response = {0},
        [EEPROM_CONTROL_2].argSize = 1,  [EEPROM_CONTROL_2].dataSize = 0,

            [EEPROM_DISCARD_FRAMING_ERRORS].callSize = 3, [EEPROM_DISCARD_FRAMING_ERRORS].call = {0x57,0x01,0x80},
            [EEPROM_DISCARD_FRAMING_ERRORS].respSize = 0, [EEPROM_DISCARD_FRAMING_ERRORS].response = {0},
            [EEPROM_DISCARD_FRAMING_ERRORS].argSize = 1,  [EEPROM_DISCARD_FRAMING_ERRORS].dataSize = 0,

            [EEPROM_HOP_PACKET_DELINEATION].callSize = 3, [EEPROM_HOP_PACKET_DELINEATION].call = {0x57,0x01,0x40},
            [EEPROM_HOP_PACKET_DELINEATION].respSize = 0, [EEPROM_HOP_PACKET_DELINEATION].response = {0},
            [EEPROM_HOP_PACKET_DELINEATION].argSize = 1,  [EEPROM_HOP_PACKET_DELINEATION].dataSize = 0,

            [EEPROM_OVERRIDE_485_TIMING].callSize = 3, [EEPROM_OVERRIDE_485_TIMING].call = {0x57,0x01,0x20},
            [EEPROM_OVERRIDE_485_TIMING].respSize = 0, [EEPROM_OVERRIDE_485_TIMING].response = {0},
            [EEPROM_OVERRIDE_485_TIMING].argSize = 1,  [EEPROM_OVERRIDE_485_TIMING].dataSize = 0,

            [EEPROM_REMOTE_ANALOG_ENABLE].callSize = 3, [EEPROM_REMOTE_ANALOG_ENABLE].call = {0x57,0x01,0x10},
            [EEPROM_REMOTE_ANALOG_ENABLE].respSize = 0, [EEPROM_REMOTE_ANALOG_ENABLE].response = {0},
            [EEPROM_REMOTE_ANALOG_ENABLE].argSize = 1,  [EEPROM_REMOTE_ANALOG_ENABLE].dataSize = 0,

            [EEPROM_REMOTE_IO_MODE].callSize = 3, [EEPROM_REMOTE_IO_MODE].call = {0x57,0x01,0x08},
            [EEPROM_REMOTE_IO_MODE].respSize = 0, [EEPROM_REMOTE_IO_MODE].response = {0},
            [EEPROM_REMOTE_IO_MODE].argSize = 1,  [EEPROM_REMOTE_IO_MODE].dataSize = 0,

            [EEPROM_RS485_DATA_ENABLE].callSize = 3, [EEPROM_RS485_DATA_ENABLE].call = {0x57,0x01,0x04},
            [EEPROM_RS485_DATA_ENABLE].respSize = 0, [EEPROM_RS485_DATA_ENABLE].response = {0},
            [EEPROM_RS485_DATA_ENABLE].argSize = 1,  [EEPROM_RS485_DATA_ENABLE].dataSize = 0,

            [EEPROM_NINE_BIT_MODE].callSize = 3, [EEPROM_NINE_BIT_MODE].call = {0x57,0x01,0x02},
            [EEPROM_NINE_BIT_MODE].respSize = 0, [EEPROM_NINE_BIT_MODE].response = {0},
            [EEPROM_NINE_BIT_MODE].argSize = 1,  [EEPROM_NINE_BIT_MODE].dataSize = 0,

            [EEPROM_9600_BOOT_OPTION].callSize = 3, [EEPROM_9600_BOOT_OPTION].call = {0x57,0x01,0x01},
            [EEPROM_9600_BOOT_OPTION].respSize = 0, [EEPROM_9600_BOOT_OPTION].response = {0},
            [EEPROM_9600_BOOT_OPTION].argSize = 1,  [EEPROM_9600_BOOT_OPTION].dataSize = 0,

        [EEPROM_INTERFACE_TIMEOUT].callSize = 2, [EEPROM_INTERFACE_TIMEOUT].call = {0x58,0x01},
        [EEPROM_INTERFACE_TIMEOUT].respSize = 0, [EEPROM_INTERFACE_TIMEOUT].response = {0},
        [EEPROM_INTERFACE_TIMEOUT].argSize = 1,  [EEPROM_INTERFACE_TIMEOUT].dataSize = 0,

        [EEPROM_ANTENNA_OVERRIDE].callSize = 2, [EEPROM_ANTENNA_OVERRIDE].call = {0x59,0x01},
        [EEPROM_ANTENNA_OVERRIDE].respSize = 0, [EEPROM_ANTENNA_OVERRIDE].response = {0},
        [EEPROM_ANTENNA_OVERRIDE].argSize = 1,  [EEPROM_ANTENNA_OVERRIDE].dataSize = 0,

        [EEPROM_RF_PACKET_SIZE].callSize = 2, [EEPROM_RF_PACKET_SIZE].call = {0x5A,0x01},
        [EEPROM_RF_PACKET_SIZE].respSize = 0, [EEPROM_RF_PACKET_SIZE].response = {0},
        [EEPROM_RF_PACKET_SIZE].argSize = 1,  [EEPROM_RF_PACKET_SIZE].dataSize = 0,

        [EEPROM_CTS_ON].callSize = 2, [EEPROM_CTS_ON].call = {0x5C,0x02},
        [EEPROM_CTS_ON].respSize = 0, [EEPROM_CTS_ON].response = {0},
        [EEPROM_CTS_ON].argSize = 2,  [EEPROM_CTS_ON].dataSize = 0,

        [EEPROM_CTS_OFF].callSize = 2, [EEPROM_CTS_OFF].call = {0x5E,0x02},
        [EEPROM_CTS_OFF].respSize = 0, [EEPROM_CTS_OFF].response = {0},
        [EEPROM_CTS_OFF].argSize = 2,  [EEPROM_CTS_OFF].dataSize = 0,

        [EEPROM_REMOTE_IO_CONTROL].callSize = 2, [EEPROM_REMOTE_IO_CONTROL].call = {0x60,0x01},
        [EEPROM_REMOTE_IO_CONTROL].respSize = 0, [EEPROM_REMOTE_IO_CONTROL].response = {0},
        [EEPROM_REMOTE_IO_CONTROL].argSize = 1,  [EEPROM_REMOTE_IO_CONTROL].dataSize = 0,

            [EEPROM_USE_PAIRS].callSize = 3, [EEPROM_USE_PAIRS].call = {0x60,0x01,0x80},
            [EEPROM_USE_PAIRS].respSize = 0, [EEPROM_USE_PAIRS].response = {0},
            [EEPROM_USE_PAIRS].argSize = 1,  [EEPROM_USE_PAIRS].dataSize = 0,

            [EEPROM_ALL_INPUTS].callSize = 3, [EEPROM_ALL_INPUTS].call = {0x60,0x01,0x40},
            [EEPROM_ALL_INPUTS].respSize = 0, [EEPROM_ALL_INPUTS].response = {0},
            [EEPROM_ALL_INPUTS].argSize = 1,  [EEPROM_ALL_INPUTS].dataSize = 0,

            [EEPROM_RXD_TXD_PAIR].callSize = 3, [EEPROM_RXD_TXD_PAIR].call = {0x60,0x01,0x20},
            [EEPROM_RXD_TXD_PAIR].respSize = 0, [EEPROM_RXD_TXD_PAIR].response = {0},
            [EEPROM_RXD_TXD_PAIR].argSize = 1,  [EEPROM_RXD_TXD_PAIR].dataSize = 0,

            [EEPROM_RTS_CTS_PAIR].callSize = 3, [EEPROM_RTS_CTS_PAIR].call = {0x60,0x01,0x10},
            [EEPROM_RTS_CTS_PAIR].respSize = 0, [EEPROM_RTS_CTS_PAIR].response = {0},
            [EEPROM_RTS_CTS_PAIR].argSize = 1,  [EEPROM_RTS_CTS_PAIR].dataSize = 0,

            [EEPROM_CMD_DATA_GIO2_PAIR].callSize = 3, [EEPROM_CMD_DATA_GIO2_PAIR].call = {0x60,0x01,0x08},
            [EEPROM_CMD_DATA_GIO2_PAIR].respSize = 0, [EEPROM_CMD_DATA_GIO2_PAIR].response = {0},
            [EEPROM_CMD_DATA_GIO2_PAIR].argSize = 1,  [EEPROM_CMD_DATA_GIO2_PAIR].dataSize = 0,

            [EEPROM_GIO7_GIO3_PAIR].callSize = 3, [EEPROM_GIO7_GIO3_PAIR].call = {0x60,0x01,0x04},
            [EEPROM_GIO7_GIO3_PAIR].respSize = 0, [EEPROM_GIO7_GIO3_PAIR].response = {0},
            [EEPROM_GIO7_GIO3_PAIR].argSize = 1,  [EEPROM_GIO7_GIO3_PAIR].dataSize = 0,

            [EEPROM_GIO8_GIO1_PAIR].callSize = 3, [EEPROM_GIO8_GIO1_PAIR].call = {0x60,0x01,0x02},
            [EEPROM_GIO8_GIO1_PAIR].respSize = 0, [EEPROM_GIO8_GIO1_PAIR].response = {0},
            [EEPROM_GIO8_GIO1_PAIR].argSize = 1,  [EEPROM_GIO8_GIO1_PAIR].dataSize = 0,

            [EEPROM_GIO4_GIO0_PAIR].callSize = 3, [EEPROM_GIO4_GIO0_PAIR].call = {0x60,0x01,0x01},
            [EEPROM_GIO4_GIO0_PAIR].respSize = 0, [EEPROM_GIO4_GIO0_PAIR].response = {0},
            [EEPROM_GIO4_GIO0_PAIR].argSize = 1,  [EEPROM_GIO4_GIO0_PAIR].dataSize = 0,

        [EEPROM_SLEEP_CONTROL].callSize = 2, [EEPROM_SLEEP_CONTROL].call = {0x61,0x01},
        [EEPROM_SLEEP_CONTROL].respSize = 0, [EEPROM_SLEEP_CONTROL].response = {0},
        [EEPROM_SLEEP_CONTROL].argSize = 1,  [EEPROM_SLEEP_CONTROL].dataSize = 0,

            [EEPROM_CYCLIC_SLEEP].callSize = 3, [EEPROM_CYCLIC_SLEEP].call = {0x61,0x01,0x01},
            [EEPROM_CYCLIC_SLEEP].respSize = 0, [EEPROM_CYCLIC_SLEEP].response = {0},
            [EEPROM_CYCLIC_SLEEP].argSize = 1,  [EEPROM_CYCLIC_SLEEP].dataSize = 0,

        [EEPROM_MAX_POWER].callSize = 2, [EEPROM_MAX_POWER].call = {0x63,0x01},
        [EEPROM_MAX_POWER].respSize = 0, [EEPROM_MAX_POWER].response = {0},
        [EEPROM_MAX_POWER].argSize = 1,  [EEPROM_MAX_POWER].dataSize = 0,

            [EEPROM_FULL_POWER].callSize = 3, [EEPROM_FULL_POWER].call = {0x63,0x01,0x00},
            [EEPROM_FULL_POWER].respSize = 0, [EEPROM_FULL_POWER].response = {0},
            [EEPROM_FULL_POWER].argSize = 0,  [EEPROM_FULL_POWER].dataSize = 0,

            [EEPROM_HALF_POWER].callSize = 3, [EEPROM_HALF_POWER].call = {0x63,0x01,0x01},
            [EEPROM_HALF_POWER].respSize = 0, [EEPROM_HALF_POWER].response = {0},
            [EEPROM_HALF_POWER].argSize = 0,  [EEPROM_HALF_POWER].dataSize = 0,

            [EEPROM_QUARTER_POWER].callSize = 3, [EEPROM_QUARTER_POWER].call = {0x63,0x01,0x02},
            [EEPROM_QUARTER_POWER].respSize = 0, [EEPROM_QUARTER_POWER].response = {0},
            [EEPROM_QUARTER_POWER].argSize = 0,  [EEPROM_QUARTER_POWER].dataSize = 0,

            [EEPROM_LOW_POWER].callSize = 3, [EEPROM_LOW_POWER].call = {0x63,0x01,0x03},
            [EEPROM_LOW_POWER].respSize = 0, [EEPROM_LOW_POWER].response = {0},
            [EEPROM_LOW_POWER].argSize = 0,  [EEPROM_LOW_POWER].dataSize = 0,

        [EEPROM_RSSI_THRESHOLD_HIGH].callSize = 2, [EEPROM_RSSI_THRESHOLD_HIGH].call = {0x65,0x01},
        [EEPROM_RSSI_THRESHOLD_HIGH].respSize = 0, [EEPROM_RSSI_THRESHOLD_HIGH].response = {0},
        [EEPROM_RSSI_THRESHOLD_HIGH].argSize = 1,  [EEPROM_RSSI_THRESHOLD_HIGH].dataSize = 0,

        [EEPROM_RSSI_THRESHOLD_LOW].callSize = 2, [EEPROM_RSSI_THRESHOLD_LOW].call = {0x66,0x01},
        [EEPROM_RSSI_THRESHOLD_LOW].respSize = 0, [EEPROM_RSSI_THRESHOLD_LOW].response = {0},
        [EEPROM_RSSI_THRESHOLD_LOW].argSize = 1,  [EEPROM_RSSI_THRESHOLD_LOW].dataSize = 0,

        [EEPROM_RSSI_LAG].callSize = 2, [EEPROM_RSSI_LAG].call = {0x67,0x01},
        [EEPROM_RSSI_LAG].respSize = 0, [EEPROM_RSSI_LAG].response = {0},
        [EEPROM_RSSI_LAG].argSize = 1,  [EEPROM_RSSI_LAG].dataSize = 0,

        [EEPROM_RSSI_CONTROL].callSize = 2, [EEPROM_RSSI_CONTROL].call = {0x68,0x01},
        [EEPROM_RSSI_CONTROL].respSize = 0, [EEPROM_RSSI_CONTROL].response = {0},
        [EEPROM_RSSI_CONTROL].argSize = 1,  [EEPROM_RSSI_CONTROL].dataSize = 0,

            [EEPROM_PWM_OUTPORT_HIGH].callSize = 3, [EEPROM_PWM_OUTPORT_HIGH].call = {0x68,0x01,0x80},
            [EEPROM_PWM_OUTPORT_HIGH].respSize = 0, [EEPROM_PWM_OUTPORT_HIGH].response = {0},
            [EEPROM_PWM_OUTPORT_HIGH].argSize = 1,  [EEPROM_PWM_OUTPORT_HIGH].dataSize = 0,

            [EEPROM_PWM_OUTPORT_LOW].callSize = 3, [EEPROM_PWM_OUTPORT_LOW].call = {0x68,0x01,0x40},
            [EEPROM_PWM_OUTPORT_LOW].respSize = 0, [EEPROM_PWM_OUTPORT_LOW].response = {0},
            [EEPROM_PWM_OUTPORT_LOW].argSize = 1,  [EEPROM_PWM_OUTPORT_LOW].dataSize = 0,

            [EEPROM_USE_AVERAGE_RSSI].callSize = 3, [EEPROM_USE_AVERAGE_RSSI].call = {0x68,0x01,0x20},
            [EEPROM_USE_AVERAGE_RSSI].respSize = 0, [EEPROM_USE_AVERAGE_RSSI].response = {0},
            [EEPROM_USE_AVERAGE_RSSI].argSize = 1,  [EEPROM_USE_AVERAGE_RSSI].dataSize = 0,

            [EEPROM_INVERT_REPORT].callSize = 3, [EEPROM_INVERT_REPORT].call = {0x68,0x01,0x10},
            [EEPROM_INVERT_REPORT].respSize = 0, [EEPROM_INVERT_REPORT].response = {0},
            [EEPROM_INVERT_REPORT].argSize = 1,  [EEPROM_INVERT_REPORT].dataSize = 0,

            [EEPROM_UNINTENDED_REPORT].callSize = 3, [EEPROM_UNINTENDED_REPORT].call = {0x68,0x01,0x08},
            [EEPROM_UNINTENDED_REPORT].respSize = 0, [EEPROM_UNINTENDED_REPORT].response = {0},
            [EEPROM_UNINTENDED_REPORT].argSize = 1,  [EEPROM_UNINTENDED_REPORT].dataSize = 0,

            [EEPROM_BROADCAST_REPORT].callSize = 3, [EEPROM_BROADCAST_REPORT].call = {0x68,0x01,0x04},
            [EEPROM_BROADCAST_REPORT].respSize = 0, [EEPROM_BROADCAST_REPORT].response = {0},
            [EEPROM_BROADCAST_REPORT].argSize = 1,  [EEPROM_BROADCAST_REPORT].dataSize = 0,

            [EEPROM_ADDRESSED_REPORT].callSize = 3, [EEPROM_ADDRESSED_REPORT].call = {0x68,0x01,0x02},
            [EEPROM_ADDRESSED_REPORT].respSize = 0, [EEPROM_ADDRESSED_REPORT].response = {0},
            [EEPROM_ADDRESSED_REPORT].argSize = 1,  [EEPROM_ADDRESSED_REPORT].dataSize = 0,

            [EEPROM_BEACON_REPORT].callSize = 3, [EEPROM_BEACON_REPORT].call = {0x68,0x01,0x01},
            [EEPROM_BEACON_REPORT].respSize = 0, [EEPROM_BEACON_REPORT].response = {0},
            [EEPROM_BEACON_REPORT].argSize = 1,  [EEPROM_BEACON_REPORT].dataSize = 0,

        [EEPROM_BEACON_SKIP].callSize = 2, [EEPROM_BEACON_SKIP].call = {0x6F,0x01},
        [EEPROM_BEACON_SKIP].respSize = 0, [EEPROM_BEACON_SKIP].response = {0},
        [EEPROM_BEACON_SKIP].argSize = 1,  [EEPROM_BEACON_SKIP].dataSize = 0,

        [EEPROM_DEST_MAC_ADDRESS].callSize = 2, [EEPROM_DEST_MAC_ADDRESS].call = {0x70,0x06},
        [EEPROM_DEST_MAC_ADDRESS].respSize = 0, [EEPROM_DEST_MAC_ADDRESS].response = {0},
        [EEPROM_DEST_MAC_ADDRESS].argSize = 6,  [EEPROM_DEST_MAC_ADDRESS].dataSize = 0,

        [EEPROM_SYSTEM_ID].callSize = 2, [EEPROM_SYSTEM_ID].call = {0x76,0x01},
        [EEPROM_SYSTEM_ID].respSize = 0, [EEPROM_SYSTEM_ID].response = {0},
        [EEPROM_SYSTEM_ID].argSize = 1,  [EEPROM_SYSTEM_ID].dataSize = 0,

        [EEPROM_MAC_ADDRESS].callSize = 2, [EEPROM_MAC_ADDRESS].call = {0x80,0x06},
        [EEPROM_MAC_ADDRESS].respSize = 0, [EEPROM_MAC_ADDRESS].response = {0},
        [EEPROM_MAC_ADDRESS].argSize = 6,  [EEPROM_MAC_ADDRESS].dataSize = 0,

        [EEPROM_PART_NUMBERS].callSize = 2, [EEPROM_PART_NUMBERS].call = {0x90,0x10},
        [EEPROM_PART_NUMBERS].respSize = 0, [EEPROM_PART_NUMBERS].response = {0},
        [EEPROM_PART_NUMBERS].argSize = 16, [EEPROM_PART_NUMBERS].dataSize = 0,

        [EEPROM_USER_MEMORY].callSize = 2, [EEPROM_USER_MEMORY].call = {0xA0,0x10},
        [EEPROM_USER_MEMORY].respSize = 0, [EEPROM_USER_MEMORY].response = {0},
        [EEPROM_USER_MEMORY].argSize = 16, [EEPROM_USER_MEMORY].dataSize = 0,

        [EEPROM_API_CONTROL].callSize = 2, [EEPROM_API_CONTROL].call = {0xC1,0x01},
        [EEPROM_API_CONTROL].respSize = 0, [EEPROM_API_CONTROL].response = {0},
        [EEPROM_API_CONTROL].argSize = 1,  [EEPROM_API_CONTROL].dataSize = 0,

            [EEPROM_BROADCAST_MODE].callSize = 3, [EEPROM_BROADCAST_MODE].call = {0xC1,0x01,0x80},
            [EEPROM_BROADCAST_MODE].respSize = 0, [EEPROM_BROADCAST_MODE].response = {0},
            [EEPROM_BROADCAST_MODE].argSize = 1,  [EEPROM_BROADCAST_MODE].dataSize = 0,

            [EEPROM_INRANGE_HIGH_ON_WAKE].callSize = 3, [EEPROM_INRANGE_HIGH_ON_WAKE].call = {0xC1,0x01,0x40},
            [EEPROM_INRANGE_HIGH_ON_WAKE].respSize = 0, [EEPROM_INRANGE_HIGH_ON_WAKE].response = {0},
            [EEPROM_INRANGE_HIGH_ON_WAKE].argSize = 1,  [EEPROM_INRANGE_HIGH_ON_WAKE].dataSize = 0,

            [EEPROM_ANTENNA_SELECT].callSize = 3, [EEPROM_ANTENNA_SELECT].call = {0xC1,0x01,0x20},
            [EEPROM_ANTENNA_SELECT].respSize = 0, [EEPROM_ANTENNA_SELECT].response = {0},
            [EEPROM_ANTENNA_SELECT].argSize = 1,  [EEPROM_ANTENNA_SELECT].dataSize = 0,

            [EEPROM_DISABLE_STATUS_BIN].callSize = 3, [EEPROM_DISABLE_STATUS_BIN].call = {0xC1,0x01,0x10},
            [EEPROM_DISABLE_STATUS_BIN].respSize = 0, [EEPROM_DISABLE_STATUS_BIN].response = {0},
            [EEPROM_DISABLE_STATUS_BIN].argSize = 1,  [EEPROM_DISABLE_STATUS_BIN].dataSize = 0,

            [EEPROM_UNICAST_ONLY].callSize = 3, [EEPROM_UNICAST_ONLY].call = {0xC1,0x01,0x08},
            [EEPROM_UNICAST_ONLY].respSize = 0, [EEPROM_UNICAST_ONLY].response = {0},
            [EEPROM_UNICAST_ONLY].argSize = 1,  [EEPROM_UNICAST_ONLY].dataSize = 0,

            [EEPROM_API_SEND_DATA_COMPLETE].callSize = 3, [EEPROM_API_SEND_DATA_COMPLETE].call = {0xC1,0x01,0x04},
            [EEPROM_API_SEND_DATA_COMPLETE].respSize = 0, [EEPROM_API_SEND_DATA_COMPLETE].response = {0},
            [EEPROM_API_SEND_DATA_COMPLETE].argSize = 1,  [EEPROM_API_SEND_DATA_COMPLETE].dataSize = 0,

            [EEPROM_API_TRANSMIT].callSize = 3, [EEPROM_API_TRANSMIT].call = {0xC1,0x01,0x02},
            [EEPROM_API_TRANSMIT].respSize = 0, [EEPROM_API_TRANSMIT].response = {0},
            [EEPROM_API_TRANSMIT].argSize = 1,  [EEPROM_API_TRANSMIT].dataSize = 0,

            [EEPROM_API_RECEIVE].callSize = 3, [EEPROM_API_RECEIVE].call = {0xC1,0x01,0x01},
            [EEPROM_API_RECEIVE].respSize = 0, [EEPROM_API_RECEIVE].response = {0},
            [EEPROM_API_RECEIVE].argSize = 1,  [EEPROM_API_RECEIVE].dataSize = 0,

        [EEPROM_RANDOM_BACKOFF].callSize = 2, [EEPROM_RANDOM_BACKOFF].call = {0xC3,0x01},
        [EEPROM_RANDOM_BACKOFF].respSize = 0, [EEPROM_RANDOM_BACKOFF].response = {0},
        [EEPROM_RANDOM_BACKOFF].argSize = 1,  [EEPROM_RANDOM_BACKOFF].dataSize = 0,

        [EEPROM_PWM_INITIALIZE].callSize = 2, [EEPROM_PWM_INITIALIZE].call = {0xC8,0x01},
        [EEPROM_PWM_INITIALIZE].respSize = 0, [EEPROM_PWM_INITIALIZE].response = {0},
        [EEPROM_PWM_INITIALIZE].argSize = 1,  [EEPROM_PWM_INITIALIZE].dataSize = 0,

        [EEPROM_REMOTE_IO_INIT_HIGH].callSize = 2, [EEPROM_REMOTE_IO_INIT_HIGH].call = {0xC9,0x01},
        [EEPROM_REMOTE_IO_INIT_HIGH].respSize = 0, [EEPROM_REMOTE_IO_INIT_HIGH].response = {0},
        [EEPROM_REMOTE_IO_INIT_HIGH].argSize = 1,  [EEPROM_REMOTE_IO_INIT_HIGH].dataSize = 0,

            [EEPROM_RXD_INITIALIZE].callSize = 3, [EEPROM_RXD_INITIALIZE].call = {0xC9,0x01,0x20},
            [EEPROM_RXD_INITIALIZE].respSize = 0, [EEPROM_RXD_INITIALIZE].response = {0},
            [EEPROM_RXD_INITIALIZE].argSize = 1,  [EEPROM_RXD_INITIALIZE].dataSize = 0,

            [EEPROM_RTS_INITIALIZE].callSize = 3, [EEPROM_RTS_INITIALIZE].call = {0xC9,0x01,0x10},
            [EEPROM_RTS_INITIALIZE].respSize = 0, [EEPROM_RTS_INITIALIZE].response = {0},
            [EEPROM_RTS_INITIALIZE].argSize = 1,  [EEPROM_RTS_INITIALIZE].dataSize = 0,

            [EEPROM_CMD_DATA_INITIALIZE].callSize = 3, [EEPROM_CMD_DATA_INITIALIZE].call = {0xC9,0x01,0x08},
            [EEPROM_CMD_DATA_INITIALIZE].respSize = 0, [EEPROM_CMD_DATA_INITIALIZE].response = {0},
            [EEPROM_CMD_DATA_INITIALIZE].argSize = 1,  [EEPROM_CMD_DATA_INITIALIZE].dataSize = 0,

            [EEPROM_GIO7_INITIALIZE].callSize = 3, [EEPROM_GIO7_INITIALIZE].call = {0xC9,0x01,0x04},
            [EEPROM_GIO7_INITIALIZE].respSize = 0, [EEPROM_GIO7_INITIALIZE].response = {0},
            [EEPROM_GIO7_INITIALIZE].argSize = 1,  [EEPROM_GIO7_INITIALIZE].dataSize = 0,

            [EEPROM_GIO8_INITIALIZE].callSize = 3, [EEPROM_GIO8_INITIALIZE].call = {0xC9,0x01,0x02},
            [EEPROM_GIO8_INITIALIZE].respSize = 0, [EEPROM_GIO8_INITIALIZE].response = {0},
            [EEPROM_GIO8_INITIALIZE].argSize = 1,  [EEPROM_GIO8_INITIALIZE].dataSize = 0,

            [EEPROM_GIO4_INITIALIZE].callSize = 3, [EEPROM_GIO4_INITIALIZE].call = {0xC9,0x01,0x01},
            [EEPROM_GIO4_INITIALIZE].respSize = 0, [EEPROM_GIO4_INITIALIZE].response = {0},
            [EEPROM_GIO4_INITIALIZE].argSize = 1,  [EEPROM_GIO4_INITIALIZE].dataSize = 0,

        [EEPROM_REMOTE_IO_INIT_LOW].callSize = 2, [EEPROM_REMOTE_IO_INIT_LOW].call = {0xCA,0x01},
        [EEPROM_REMOTE_IO_INIT_LOW].respSize = 0, [EEPROM_REMOTE_IO_INIT_LOW].response = {0},
        [EEPROM_REMOTE_IO_INIT_LOW].argSize = 1,  [EEPROM_REMOTE_IO_INIT_LOW].dataSize = 0,

            [EEPROM_TXD_INITIALIZE].callSize = 3, [EEPROM_TXD_INITIALIZE].call = {0xCA,0x01,0x20},
            [EEPROM_TXD_INITIALIZE].respSize = 0, [EEPROM_TXD_INITIALIZE].response = {0},
            [EEPROM_TXD_INITIALIZE].argSize = 1,  [EEPROM_TXD_INITIALIZE].dataSize = 0,

            [EEPROM_CTS_INITIALIZE].callSize = 3, [EEPROM_CTS_INITIALIZE].call = {0xCA,0x01,0x10},
            [EEPROM_CTS_INITIALIZE].respSize = 0, [EEPROM_CTS_INITIALIZE].response = {0},
            [EEPROM_CTS_INITIALIZE].argSize = 1,  [EEPROM_CTS_INITIALIZE].dataSize = 0,

            [EEPROM_GIO2_INITIALIZE].callSize = 3, [EEPROM_GIO2_INITIALIZE].call = {0xCA,0x01,0x08},
            [EEPROM_GIO2_INITIALIZE].respSize = 0, [EEPROM_GIO2_INITIALIZE].response = {0},
            [EEPROM_GIO2_INITIALIZE].argSize = 1,  [EEPROM_GIO2_INITIALIZE].dataSize = 0,

            [EEPROM_GIO3_INITIALIZE].callSize = 3, [EEPROM_GIO3_INITIALIZE].call = {0xCA,0x01,0x04},
            [EEPROM_GIO3_INITIALIZE].respSize = 0, [EEPROM_GIO3_INITIALIZE].response = {0},
            [EEPROM_GIO3_INITIALIZE].argSize = 1,  [EEPROM_GIO3_INITIALIZE].dataSize = 0,

            [EEPROM_GIO1_INITIALIZE].callSize = 3, [EEPROM_GIO1_INITIALIZE].call = {0xCA,0x01,0x02},
            [EEPROM_GIO1_INITIALIZE].respSize = 0, [EEPROM_GIO1_INITIALIZE].response = {0},
            [EEPROM_GIO1_INITIALIZE].argSize = 1,  [EEPROM_GIO1_INITIALIZE].dataSize = 0,

            [EEPROM_GIO0_INITIALIZE].callSize = 3, [EEPROM_GIO0_INITIALIZE].call = {0xCA,0x01,0x01},
            [EEPROM_GIO0_INITIALIZE].respSize = 0, [EEPROM_GIO0_INITIALIZE].response = {0},
            [EEPROM_GIO0_INITIALIZE].argSize = 1,  [EEPROM_GIO0_INITIALIZE].dataSize = 0,

        [EEPROM_SLEEP_TIME_HIGH].callSize = 2, [EEPROM_SLEEP_TIME_HIGH].call = {0xCD,0x01},
        [EEPROM_SLEEP_TIME_HIGH].respSize = 0, [EEPROM_SLEEP_TIME_HIGH].response = {0},
        [EEPROM_SLEEP_TIME_HIGH].argSize = 1,  [EEPROM_SLEEP_TIME_HIGH].dataSize = 0,

        [EEPROM_SLEEP_TIME_LOW].callSize = 2, [EEPROM_SLEEP_TIME_LOW].call = {0xCE,0x01},
        [EEPROM_SLEEP_TIME_LOW].respSize = 0, [EEPROM_SLEEP_TIME_LOW].response = {0},
        [EEPROM_SLEEP_TIME_LOW].argSize = 1,  [EEPROM_SLEEP_TIME_LOW].dataSize = 0,

        [EEPROM_WAKE_COUNT].callSize = 2, [EEPROM_WAKE_COUNT].call = {0xCF,0x01},
        [EEPROM_WAKE_COUNT].respSize = 0, [EEPROM_WAKE_COUNT].response = {0},
        [EEPROM_WAKE_COUNT].argSize = 1,  [EEPROM_WAKE_COUNT].dataSize = 0,

        [EEPROM_DATE_OF_BIRTH].callSize = 2, [EEPROM_DATE_OF_BIRTH].call = {0xE0,0x04},
        [EEPROM_DATE_OF_BIRTH].respSize = 0, [EEPROM_DATE_OF_BIRTH].response = {0},
        [EEPROM_DATE_OF_BIRTH].argSize = 4,  [EEPROM_DATE_OF_BIRTH].dataSize = 0,
};



/*****************************************************
 *  PRIVATE FUNCTIONS
 *****************************************************/

#ifndef EMBEDDED    /* Skip these if on embedded processors */

/*
 *  Reads from the given serial port
 *
 *  INPUTS: device - the handle for the connected port
 *          buffer - the data will be read into this pointer
 *          length - number of bytes to read
 *
 *  OUTPUTS: int - number of bytes read or -1 on error
 */
BYTE* Read(const HANDLE device, int length)
{
    BYTE *buffer = malloc(length * sizeof(BYTE));
    int bytesRead;

    #ifdef __unix__

        bytesRead = read(device, buffer, length);

        if (bytesRead >= 0)
        {
            return buffer;
        }

    #elif defined _WIN32

        if (ReadFile(device, buffer, length, (LPDWORD)((void *)&bytesRead), NULL))
        {
            if (bytesRead == length)
            {
                return buffer;
            }
        }

    #endif

    buffer = NULL;
    return buffer;
}


/*
 *  Writes to the given serial port
 *
 *  INPUTS: device - the handle for the connected serial port
 *          buffer - a pointer to the data to write
 *          length - number of bytes to write
 *
 *  OUTPUTS: int - number of bytes written or -1 on error
 */
int Write(const HANDLE device, const BYTE *buffer, int length)
{
    int bytesWritten;

    #ifdef __unix__

        return(write(file, buffer, length));

    #elif defined _WIN32

        if (WriteFile(device, buffer, length, (LPDWORD)((void *)&bytesWritten), NULL))
        {
          return bytesWritten;
        }

    #endif

    return(-1);
}


/*
 *  Delays processing for the given time
 *
 *  INPUTS: time - amount of time to delay in milliseconds
 *
 *  OUTPUTS: none
 */
void delay(double time)
{
    #ifdef __unix__
        usleep(time);
    #elif defined _WIN32
        Sleep(time);
    #endif
}

#endif /* EMBEDDED */



/*
 *  Constructs proper calls/responses in special cases
 *
 *  INPUTS: device - the handle for the connected serial port
 *          command - a valid command keyword from the COMMAND list
 *          ATCommandIn - pointer to an ATCommand struct
 *          input - pointer to the optional arguments
 *
 *  OUTPUTS: modifies ATCommandIn
 */
void alignWrite(const HANDLE device, COMMAND command, ATCommand *ATCommandIn, BYTE *input)
{
    memcpy(ATCommandIn->finalCall, ATCommandIn->call, ATCommandIn->callSize);
    memcpy(ATCommandIn->finalResp, ATCommandIn->response, ATCommandIn->respSize);

    if (command > WRITE_IRAM && command <= IRAM_SYSTEM_ID)
    {
        ATCommandIn->dataSize = RM024[WRITE_IRAM].dataSize;
        ATCommandIn->respSize = RM024[WRITE_IRAM].respSize;
        ATCommandIn->finalResp[0] = RM024[WRITE_IRAM].response[0];

        // Concatenate response
        ATCommandIn->finalResp = realloc(ATCommandIn->finalResp,(ATCommandIn->respSize + ATCommandIn->callSize + ATCommandIn->argSize) * sizeof(BYTE));
        memcpy(ATCommandIn->finalResp + ATCommandIn->respSize, ATCommandIn->call, ATCommandIn->callSize);
        memcpy(ATCommandIn->finalResp + (ATCommandIn->respSize + ATCommandIn->callSize), input, ATCommandIn->argSize);

        // Concatenate Write IRAM call header with the particular command call
        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + 2) * sizeof(BYTE));
        memcpy(ATCommandIn->finalCall + 2, ATCommandIn->call, ATCommandIn->callSize);
        memcpy(ATCommandIn->finalCall, RM024[WRITE_IRAM].call, 2);

        ATCommandIn->callSize += 2;
    }

    if (command == AUTO_CHANNEL || command == AUTO_DESTINATION)
    {
        // Only change one bit in the control byte
        if (input[0] == ENABLE)
        {
            ATCommandIn->finalCall[2] = ATCommandIn->finalCall[2] & 0xFF;
        }
        else
        {
            ATCommandIn->finalCall[2] = ATCommandIn->finalCall[2] & 0xFC;
        }

        ATCommandIn->argSize = 0;
    }

    if (command > WRITE_API_CONTROL && command <= API_RECIEVE)
    {
        BYTE *tempPtr = malloc(2*sizeof(BYTE));

        // Concatenate Write API Control call header with the particular command call
        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + 2) * sizeof(BYTE));
        memcpy(ATCommandIn->finalCall + 2, ATCommandIn->call, ATCommandIn->callSize);
        memcpy(ATCommandIn->finalCall, RM024[WRITE_API_CONTROL].call, 2);

        ATCommandIn->callSize += 2;

        // Get the current control byte to prevent overwrite
        Write(device, RM024[READ_API_CONTROL].call, 2);
        delay(40);
        tempPtr = Read(device, 2);
        delay(40);

        // Only change one bit in the control byte by masking
        if (input[0] == ENABLE)
        {
            ATCommandIn->finalCall[2] = ATCommandIn->finalCall[2] | tempPtr[1];
        }
        else
        {
            ATCommandIn->finalCall[2] = (~ATCommandIn->finalCall[2]) & tempPtr[1];
        }

        free((void*)tempPtr);
        ATCommandIn->argSize = 0;
    }

    if (command == DIGITAL_IO_0 || command == DIGITAL_IO_1)
    {
        BYTE *tempPtr = malloc(2*sizeof(BYTE));

        // Concatenate Write API Control call header with the particular command call
        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + 2) * sizeof(BYTE));
        memcpy(ATCommandIn->finalCall + 2, ATCommandIn->call, ATCommandIn->callSize);
        memcpy(ATCommandIn->finalCall, RM024[WRITE_DIGITAL_OUT].call, 2);

        ATCommandIn->callSize += 2;

        // Get the current control byte to prevent overwrite
        Write(device, RM024[READ_DIGITAL_INPUT].call, 2);
        delay(40);
        tempPtr = Read(device, 2);
        delay(40);

        // Only change one bit in the control byte by masking
        if (input[0] == ENABLE)
        {
            ATCommandIn->finalCall[2] = ATCommandIn->finalCall[2] | tempPtr[1];
        }
        else
        {
            ATCommandIn->finalCall[2] = (~ATCommandIn->finalCall[2]) & tempPtr[1];
        }

        free((void*)tempPtr);
        ATCommandIn->argSize = 0;
    }

    if (command == WRITE_FLASH)
    {
        // Get the number of remaining args from the 3rd and 4th arguments
        ATCommandIn->argSize += (input[3] | (input[2] << 8));
        // Concatenate response
        ATCommandIn->finalResp = realloc(ATCommandIn->finalResp,(ATCommandIn->respSize + 3) * sizeof(BYTE));
        ATCommandIn->finalResp[3] = input[0];
        ATCommandIn->finalResp[4] = input[1];
    }

    // Concatenate to make the final version
    ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + ATCommandIn->argSize) * sizeof(BYTE));
    memcpy(ATCommandIn->finalCall + ATCommandIn->callSize, input, ATCommandIn->argSize);
}


/*
 *  Constructs proper calls/responses in special cases
 *
 *  INPUTS: command - a valid command keyword from the COMMAND list
 *          ATCommandIn - pointer to an ATCommand struct
 *          input - pointer to the optional arguments
 *
 *  OUTPUTS: modifies ATCommandIn
 */
void alignRead(COMMAND command, ATCommand *ATCommandIn, BYTE *input)
{
    memcpy(ATCommandIn->finalCall, ATCommandIn->call, ATCommandIn->callSize);
    memcpy(ATCommandIn->finalResp, ATCommandIn->response, ATCommandIn->respSize);

    if (command > WRITE_IRAM && command <= IRAM_SYSTEM_ID)
    {
        ATCommandIn->dataSize = RM024[READ_IRAM].dataSize;
        ATCommandIn->respSize = RM024[READ_IRAM].respSize;
        ATCommandIn->callSize += 2;
        ATCommandIn->argSize = 0;

        ATCommandIn->finalResp = realloc(ATCommandIn->finalResp,(ATCommandIn->respSize) * sizeof(BYTE));
        ATCommandIn->finalResp[0] = RM024[READ_IRAM].response[0];

        // Concatenate Read IRAM call header with the particular command call
        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize) * sizeof(BYTE));
        memcpy(ATCommandIn->finalCall + 2, ATCommandIn->call, ATCommandIn->callSize);
        memcpy(ATCommandIn->finalCall, RM024[READ_IRAM].call, 2);
    }

    if (command == AUTO_CHANNEL || command == AUTO_DESTINATION)
    {
        ATCommandIn->finalCall[2] = 0x00;
    }

    if (command > WRITE_API_CONTROL && command <= API_RECIEVE)
    {
        ATCommandIn->callSize = 2;
        ATCommandIn->argSize = 0;

        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize) * sizeof(BYTE));
        ATCommandIn->finalCall[0] = RM024[READ_API_CONTROL].call[0];
        ATCommandIn->finalCall[1] = RM024[READ_API_CONTROL].call[1];
    }

    if (command == DIGITAL_IO_0 || command == DIGITAL_IO_1)
    {
        ATCommandIn->callSize = 2;
        ATCommandIn->argSize = 0;

        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize) * sizeof(BYTE));
        ATCommandIn->finalCall[0] = RM024[READ_DIGITAL_INPUT].call[0];
        ATCommandIn->finalCall[1] = RM024[READ_DIGITAL_INPUT].call[1];
    }

    if (command == READ_FLASH)
    {
        // Get the number of remaining args from the 3rd and 4th arguments
        ATCommandIn->dataSize += (input[3] | (input[2] << 8));
        // Concatenate response
        ATCommandIn->finalResp = realloc(ATCommandIn->finalResp,(ATCommandIn->respSize + 5) * sizeof(BYTE));
        ATCommandIn->finalResp[3] = input[0];
        ATCommandIn->finalResp[4] = input[1];
        ATCommandIn->finalResp[5] = input[2];
        ATCommandIn->finalResp[6] = input[3];
    }

    // Concatenate to create the final form
    ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + ATCommandIn->argSize) * sizeof(BYTE));
    memcpy(ATCommandIn->finalCall + ATCommandIn->callSize, input, ATCommandIn->argSize);
}


/*
 *  Rearranges the bytes for the special cases of writing certain commands
 *      to the RM024 EEPROM
 *
 *  INPUTS: device - the handle for the connected serial port
 *          command - a valid command keyword from the COMMAND list
 *          ATCommandIn - pointer to an ATCommand struct
 *          input - pointer to the optional arguments
 *
 *  OUTPUTS: modifies ATCommandIn
 */
void alignWrite_EEPROM(const HANDLE device, EEPROM command, ATCommand *ATCommandIn, BYTE *input)
{
    memcpy(ATCommandIn->finalCall, ATCommandIn->call, ATCommandIn->callSize);
    memcpy(ATCommandIn->finalResp, ATCommandIn->response, ATCommandIn->respSize);

    if (command == EEPROM_BYTE_WRITE)
    {
        ATCommandIn->argSize += input[1];
    }

    if (command > EEPROM_BYTE_WRITE)
    {

        ATCommandIn->finalResp = realloc(ATCommandIn->finalResp,(ATCommandIn->respSize + 3) * sizeof(BYTE));
        memcpy(ATCommandIn->finalResp, ATCommandIn->call, 3);
        ATCommandIn->respSize = 2;
        ATCommandIn->dataSize = 1;

        // Concatenate Write EEPROM call header with the particular command call
        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + 2) * sizeof(BYTE));
        memcpy(ATCommandIn->finalCall + 2, ATCommandIn->call, ATCommandIn->callSize);
        memcpy(ATCommandIn->finalCall, EEPROM024[EEPROM_BYTE_WRITE].call, 2);

        ATCommandIn->callSize += 2;
    }

    if ((command > EEPROM_CONTROL_0 && command <= EEPROM_SNIFF_PERMIT)
            || (command > EEPROM_CONTROL_1 && command <= EEPROM_AUTO_CONFIG)
            || (command > EEPROM_CONTROL_2 && command <= EEPROM_9600_BOOT_OPTION)
            || (command > EEPROM_REMOTE_IO_CONTROL && command <= EEPROM_GIO4_GIO0_PAIR)
            || (command > EEPROM_SLEEP_CONTROL && command <= EEPROM_CYCLIC_SLEEP)
            || (command > EEPROM_RSSI_CONTROL && command <= EEPROM_BEACON_REPORT)
            || (command > EEPROM_API_CONTROL && command <= EEPROM_API_RECEIVE)
            || (command > EEPROM_REMOTE_IO_INIT_HIGH && command <= EEPROM_GIO4_INITIALIZE)
            || (command > EEPROM_REMOTE_IO_INIT_LOW && command <= EEPROM_GIO0_INITIALIZE))
    {
        BYTE temp[4];
        BYTE *tempPtr = malloc(4*sizeof(BYTE));

        temp[0] = EEPROM024[EEPROM_BYTE_READ].call[0];
        temp[1] = EEPROM024[EEPROM_BYTE_READ].call[1];
        temp[2] = EEPROM024[command].call[0];
        temp[3] = EEPROM024[command].call[1];

        // Get the current control byte to prevent overwrite
        Write(device, temp, 4);
        delay(50);
        tempPtr = Read(device, 4);
        delay(40);

        // Only change one bit in the control byte by masking
        if (input[0] == ENABLE)
        {
            ATCommandIn->finalCall[4] = ATCommandIn->finalCall[4] | tempPtr[3];
        }
        else
        {
            ATCommandIn->finalCall[4] = (~ATCommandIn->finalCall[4]) & tempPtr[3];
        }

        free((void*)tempPtr);
        ATCommandIn->argSize = 0;
    }

    if (command == EEPROM_ANTENNA_OVERRIDE)
    {
        if (input[0] == ENABLE)
        {
            ATCommandIn->finalCall[4] = 0xE3;
        }
        else
        {
            ATCommandIn->finalCall[4] = 0xFF;
        }
        ATCommandIn->argSize = 0;
    }

    // Concatenate to make the final version
    ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + ATCommandIn->argSize) * sizeof(BYTE));
    memcpy(ATCommandIn->finalCall + ATCommandIn->callSize, input, ATCommandIn->argSize);
}


/*
 *  Rearranges the bytes for the special cases of using certain commands
 *      for reading from the RM024 EEPROM
 *
 *  INPUTS: command - a valid command keyword from the COMMAND list
 *          ATCommandIn - pointer to an ATCommand struct
 *          input - pointer to the optional arguments
 *
 *  OUTPUTS: modifies ATCommandIn
 */
void alignRead_EEPROM(EEPROM command, ATCommand *ATCommandIn, BYTE *input)
{
    memcpy(ATCommandIn->finalCall, ATCommandIn->call, ATCommandIn->callSize);
    memcpy(ATCommandIn->finalResp, ATCommandIn->response, ATCommandIn->respSize);

    if (command == EEPROM_BYTE_READ)
    {
        ATCommandIn->finalResp = realloc(ATCommandIn->finalResp,(ATCommandIn->respSize + 2) * sizeof(BYTE));
        ATCommandIn->finalResp[1] = input[0];
        ATCommandIn->finalResp[2] = input[1];
        ATCommandIn->respSize = 3;
        ATCommandIn->dataSize = input[1];
    }

    if (command > EEPROM_BYTE_WRITE)
    {
        ATCommandIn->finalResp = realloc(ATCommandIn->finalResp,(ATCommandIn->respSize + 3) * sizeof(BYTE));
        ATCommandIn->finalResp[0] = EEPROM024[EEPROM_BYTE_READ].response[0];
        ATCommandIn->finalResp[1] = ATCommandIn->call[0];
        ATCommandIn->finalResp[2] = ATCommandIn->call[1];
        ATCommandIn->respSize = 3;
        ATCommandIn->dataSize = ATCommandIn->call[1];
        ATCommandIn->argSize = 0;

        // Concatenate Read EEPROM call header with the particular command call
        ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + 2) * sizeof(BYTE));
        memcpy(ATCommandIn->finalCall + 2, ATCommandIn->call, ATCommandIn->callSize);
        memcpy(ATCommandIn->finalCall, EEPROM024[EEPROM_BYTE_READ].call, 2);

        ATCommandIn->callSize += 2;
    }

    // Concatenate to make the final version
    ATCommandIn->finalCall = realloc(ATCommandIn->finalCall,(ATCommandIn->callSize + ATCommandIn->argSize) * sizeof(BYTE));
    memcpy(ATCommandIn->finalCall + ATCommandIn->callSize, input, ATCommandIn->argSize);
}


/*****************************************************
 *  PUBLIC FUNCTIONS
 *****************************************************/

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
ERR WriteRM024(const HANDLE device, COMMAND command, ...)
{
    // Variable declarations
    ATCommand ATCommandIn = RM024[command];
    ERR returnVal;

    va_list argsList;
    BYTE *args = NULL;
    BYTE *response = NULL;

    ATCommandIn.finalCall = malloc((ATCommandIn.callSize) * sizeof(BYTE));
    ATCommandIn.finalResp = malloc((ATCommandIn.respSize) * sizeof(BYTE));

    //Check if the chosen command is part of the list
    if (command >= COMMAND_LENGTH)
    {
        returnVal = UNRECOGNIZED_COMMAND;
        goto CLEANUP;
    }

    // Gather extra arguments
    va_start(argsList, command);
    args = va_arg(argsList, BYTE*);
    va_end(argsList);

    // Stitch together a proper command to send to the device
    alignWrite(device, command, &ATCommandIn, args);

    // Send our command to the device
    if (Write(device, ATCommandIn.finalCall, ATCommandIn.callSize + ATCommandIn.argSize) < 0)
    {
        returnVal = COMMUNICATION_ERROR;
        goto CLEANUP;
    }

    delay(50);
    if (command == WRITE_FLASH && ATCommandIn.finalCall[2] == 0x08)
        delay(300);
    if (command == DECRYPT_NEW_IMAGE)
        delay(600);

    // Look for a response from the device if it is expected
    if (ATCommandIn.respSize != 0)
    {
        response = Read(device, ATCommandIn.respSize + ATCommandIn.dataSize);

        // No response
        if (response == NULL)
        {
            returnVal = COMMUNICATION_ERROR;
            goto CLEANUP;
        }

        // Found response, check if it is the right one
        if (memcmp(ATCommandIn.finalResp, response, ATCommandIn.respSize) != 0)
        {
            returnVal = UNRECOGNIZED_RESPONSE;
            goto CLEANUP;
        }
    }

    // Everything is fine
    returnVal = NO_ERRORS;

    // Deallocate pointers and return error code
    CLEANUP:
        free((void*)response);
        free((void*)ATCommandIn.finalCall);
        free((void*)ATCommandIn.finalResp);
        return returnVal;
}

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
ERR ReadRM024(BYTE *responseOut, const HANDLE device, COMMAND command, ...)
{
    // Variable declarations
    ATCommand ATCommandIn = RM024[command];
    ERR returnVal;

    va_list argsList;
    BYTE *args = NULL;
    BYTE *response = NULL;
    int index;

    ATCommandIn.finalCall = malloc((ATCommandIn.callSize) * sizeof(BYTE));
    ATCommandIn.finalResp = malloc((ATCommandIn.respSize) * sizeof(BYTE));

    //Check if the chosen command is part of the list
    if (command >= COMMAND_LENGTH)
    {
        returnVal = UNRECOGNIZED_COMMAND;
        goto CLEANUP;
    }

    // Gather extra arguments
    va_start(argsList, command);
    args = va_arg(argsList, BYTE*);
    va_end(argsList);

    // Stitch together a proper command to send to the device
    alignRead(command, &ATCommandIn, args);

    // Send our command to the device
    if (Write(device, ATCommandIn.finalCall, ATCommandIn.callSize + ATCommandIn.argSize) < 0)
    {
        returnVal = COMMUNICATION_ERROR;
        goto CLEANUP;
    }

    delay(50);

    // Look for a response from the device if it is expected
    if (ATCommandIn.respSize != 0)
    {
        response = Read(device, ATCommandIn.respSize + ATCommandIn.dataSize);

        // No response
        if (response == NULL)
        {
            returnVal = COMMUNICATION_ERROR;
            goto CLEANUP;
        }

        // Found response, check if it is the right one
        if (memcmp(ATCommandIn.finalResp, response, ATCommandIn.respSize) != 0)
        {
            returnVal = UNRECOGNIZED_RESPONSE;
            goto CLEANUP;
        }
    }

    // Everything is fine, return response
    for (index = 0; index < ATCommandIn.dataSize; index++)
    {
        responseOut[index] = response[ATCommandIn.respSize + index];
    }

    // Special cases for single bit responses
    if (command == AUTO_CHANNEL || command == AUTO_DESTINATION)
    {
        responseOut[0] = (responseOut[0] & ATCommandIn.call[2]) != 0;
    }
    if ((command > WRITE_API_CONTROL && command <= API_RECIEVE)
            || (command == DIGITAL_IO_0 || command == DIGITAL_IO_1))
    {
        responseOut[0] = (responseOut[0] & ATCommandIn.call[0]) != 0;
    }

    returnVal = NO_ERRORS;

    // Deallocate pointers and return error code
    CLEANUP:
        free((void*)response);
        free((void*)ATCommandIn.finalCall);
        free((void*)ATCommandIn.finalResp);
        return returnVal;
}

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
ERR WriteRM024_EEPROM(const HANDLE device, EEPROM command, ...)
{
    // Variable declarations
    ATCommand ATCommandIn = EEPROM024[command];
    ERR returnVal;

    va_list argsList;
    BYTE *args = NULL;
    BYTE *response = NULL;

    ATCommandIn.finalCall = malloc((ATCommandIn.callSize) * sizeof(BYTE));
    ATCommandIn.finalResp = malloc((ATCommandIn.respSize) * sizeof(BYTE));

    //Check if the chosen command is part of the list
    if (command < EEPROM_BYTE_WRITE || command >= EEPROM_LENGTH || command == EEPROM_PRODUCT_ID || command == EEPROM_DATE_OF_BIRTH)
    {
        returnVal = UNRECOGNIZED_COMMAND;
        goto CLEANUP;
    }

    // Gather extra arguments
    va_start(argsList, command);
    args = va_arg(argsList, BYTE*);
    va_end(argsList);

    // Stitch together a proper command to send to the device
    alignWrite_EEPROM(device, command, &ATCommandIn, args);

    // Send our command to the device
    if (Write(device, ATCommandIn.finalCall, ATCommandIn.callSize + ATCommandIn.argSize) < 0)
    {
        returnVal = COMMUNICATION_ERROR;
        goto CLEANUP;
    }

    delay(50);

    // Look for a response from the device if it is expected
    if (ATCommandIn.respSize != 0)
    {
        response = Read(device, ATCommandIn.respSize + ATCommandIn.dataSize);

        // No response
        if (response == NULL)
        {
            returnVal = COMMUNICATION_ERROR;
            goto CLEANUP;
        }

        // Found response, check if it is the right one
        if (memcmp(ATCommandIn.finalResp, response, ATCommandIn.respSize) != 0)
        {
            returnVal = UNRECOGNIZED_RESPONSE;
            goto CLEANUP;
        }
    }

    // Everything is fine
    returnVal = NO_ERRORS;

    // Deallocate pointers and return error code
    CLEANUP:
        free((void*)response);
        free((void*)ATCommandIn.finalCall);
        free((void*)ATCommandIn.finalResp);
        return returnVal;
}

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
ERR ReadRM024_EEPROM(BYTE *responseOut, const HANDLE device, EEPROM command, ...)
{
    // Variable declarations
    ATCommand ATCommandIn = EEPROM024[command];
    ERR returnVal;

    va_list argsList;
    BYTE *args = NULL;
    BYTE *response = NULL;
    int index;

    ATCommandIn.finalCall = malloc((ATCommandIn.callSize) * sizeof(BYTE));
    ATCommandIn.finalResp = malloc((ATCommandIn.respSize) * sizeof(BYTE));

    //Check if the chosen command is part of the list
    if (command >= EEPROM_LENGTH || command == EEPROM_BYTE_WRITE)
    {
        returnVal = UNRECOGNIZED_COMMAND;
        goto CLEANUP;
    }

    // Gather extra arguments
    va_start(argsList, command);
    args = va_arg(argsList, BYTE*);
    va_end(argsList);

    // Stitch together a proper command to send to the device
    alignRead_EEPROM(command, &ATCommandIn, args);

    // Send our command to the device
    if (Write(device, ATCommandIn.finalCall, ATCommandIn.callSize + ATCommandIn.argSize) < 0)
    {
        returnVal = COMMUNICATION_ERROR;
        goto CLEANUP;
    }

    delay(50);

    // Look for a response from the device if it is expected
    if (ATCommandIn.respSize != 0)
    {
        response = Read(device, ATCommandIn.respSize + ATCommandIn.dataSize);

        // No response
        if (response == NULL)
        {
            returnVal = COMMUNICATION_ERROR;
            goto CLEANUP;
        }

        // Found response, check if it is the right one
        if (memcmp(ATCommandIn.finalResp, response, ATCommandIn.respSize) != 0)
        {
            returnVal = UNRECOGNIZED_RESPONSE;
            goto CLEANUP;
        }
    }

    // Everything is fine, return response
    for (index = 0; index < ATCommandIn.dataSize; index++)
    {
        responseOut[index] = response[ATCommandIn.respSize + index];
    }

    // Special cases for single bit responses
    if ((command > EEPROM_CONTROL_0 && command <= EEPROM_SNIFF_PERMIT)
            || (command > EEPROM_CONTROL_1 && command <= EEPROM_AUTO_CONFIG)
            || (command > EEPROM_CONTROL_2 && command <= EEPROM_9600_BOOT_OPTION)
            || (command > EEPROM_REMOTE_IO_CONTROL && command <= EEPROM_GIO4_GIO0_PAIR)
            || (command > EEPROM_SLEEP_CONTROL && command <= EEPROM_CYCLIC_SLEEP)
            || (command > EEPROM_RSSI_CONTROL && command <= EEPROM_BEACON_REPORT)
            || (command > EEPROM_API_CONTROL && command <= EEPROM_API_RECEIVE)
            || (command > EEPROM_REMOTE_IO_INIT_HIGH && command <= EEPROM_GIO4_INITIALIZE)
            || (command > EEPROM_REMOTE_IO_INIT_LOW && command <= EEPROM_GIO0_INITIALIZE))
    {
        responseOut[0] = (responseOut[0] & ATCommandIn.call[2]) != 0;
    }

    returnVal = NO_ERRORS;

    // Deallocate pointers and return error code
    CLEANUP:
        free((void*)response);
        free((void*)ATCommandIn.finalCall);
        free((void*)ATCommandIn.finalResp);
        return returnVal;
}

/*
 *  Converts the given time to three bytes that can be understood by the RM024
 *   SLEEP_W_TIMER function.
 *
 *  INPUTS: time - amount of time to sleep, must be between
 *                  13 milliseconds and 18.2 hours
 *          base - a timebase for the given time. Options are MS, S, M, H
 *                  for milliseconds, seconds, minutes, hours
 *          sleepArgs - pointer for returning properly formatted
 *                                              arguments for the SLEEP_W_TIMER command
 *
 *  OUTPUTS: ERR - error code
 */
ERR FormatSleepTime(unsigned int time, TIMEBASE base, BYTE *sleepArgs)
{
    float units;

    // Set time base
    switch (base)
    {
    case MS:
        units = time / 0.030517578125;
        break;

    case S:
        units = time * 32768;
        break;

    case M:
        units = time * 32768 * 60;
        break;

    case H:
        units = time * 32768 * 60 * 60;
        break;

    default:
        return INVALID_ARGUMENT;
    }

    // Determine resolution
    if (units <= 65535)
    {
        sleepArgs[0] = 0;
    }
    else if (units > 65535 && units <= 65535*32)
    {
        units = units / 32;
        sleepArgs[0] = 1;
    }
    else if (units > 65535*32 && units <= 65535*1024)
    {
        units = units / 1024;
        sleepArgs[0] = 2;
    }
    else if (units > 65535*1024 && units <= 65535*32768)
    {
        units = units / 32768;
        sleepArgs[0] = 3;
    }
    else
    {
        return TIME_TOO_LARGE;
    }

    // Determine Time High
    sleepArgs[1] = (BYTE)((int)units >> 8);

    // Determine Time Low
    sleepArgs[2] = (BYTE)((int)units & 0x00FF);

    return NO_ERRORS;
}
