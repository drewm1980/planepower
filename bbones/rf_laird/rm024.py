#!/usr/bin/env python
print "This is a library for talking to rm024 modules with python"

"""
    [ENTER_COMMAND_MODE].callSize = 6, [ENTER_COMMAND_MODE].call = {0x41,0x54,0x2B,0x2B,0x2B,0x0D},
    [ENTER_COMMAND_MODE].respSize = 4, [ENTER_COMMAND_MODE].response = {0xCC,0x43,0x4F,0x4D},
    [ENTER_COMMAND_MODE].argSize = 0,  [ENTER_COMMAND_MODE].dataSize = 0,

    [EXIT_COMMAND_MODE].callSize = 5, [EXIT_COMMAND_MODE].call = {0xCC,0x41,0x54,0x4F,0x0D},
    [EXIT_COMMAND_MODE].respSize = 4, [EXIT_COMMAND_MODE].response = {0xCC,0x44,0x41,0x54},
    [EXIT_COMMAND_MODE].argSize = 0,  [EXIT_COMMAND_MODE].dataSize = 0,

    [STATUS_REQUEST].callSize = 3, [STATUS_REQUEST].call = {0xCC,0x00,0x00},
    [STATUS_REQUEST].respSize = 1, [STATUS_REQUEST].response = {0xCC},
    [STATUS_REQUEST].argSize = 0,  [STATUS_REQUEST].dataSize = 2,

    [CHECK_STATUS_REG].callSize = 3, [CHECK_STATUS_REG].call = {0xCC,0x00,0x01},
    [CHECK_STATUS_REG].respSize = 1, [CHECK_STATUS_REG].response = {0xCC},
    [CHECK_STATUS_REG].argSize = 0,  [CHECK_STATUS_REG].dataSize = 4,

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

    [WRITE_DEST_ADDR].callSize = 2, [WRITE_DEST_ADDR].call = {0xCC,0x10},
    [WRITE_DEST_ADDR].respSize = 1, [WRITE_DEST_ADDR].response = {0xCC},
    [WRITE_DEST_ADDR].argSize = 3,  [WRITE_DEST_ADDR].dataSize = 3,

    [READ_DEST_ADDR].callSize = 2, [READ_DEST_ADDR].call = {0xCC,0x11},
    [READ_DEST_ADDR].respSize = 1, [READ_DEST_ADDR].response = {0xCC},
    [READ_DEST_ADDR].argSize = 0,  [READ_DEST_ADDR].dataSize = 3,

    [GET_LAST_RSSI].callSize = 2, [GET_LAST_RSSI].call = {0xCC,0x22},
    [GET_LAST_RSSI].respSize = 1, [GET_LAST_RSSI].response = {0xCC},
    [GET_LAST_RSSI].argSize = 0,  [GET_LAST_RSSI].dataSize = 1,

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
	"""

"""------------- EEPROM stuff was in a different C struct
    [EEPROM_BYTE_READ].callSize = 2, [EEPROM_BYTE_READ].call = {0xCC,0xC0},
    [EEPROM_BYTE_READ].respSize = 1, [EEPROM_BYTE_READ].response = {0xCC},
    [EEPROM_BYTE_READ].argSize = 2,  [EEPROM_BYTE_READ].dataSize = 2,

    [EEPROM_BYTE_WRITE].callSize = 2, [EEPROM_BYTE_WRITE].call = {0xCC,0xC1},
    [EEPROM_BYTE_WRITE].respSize = 2, [EEPROM_BYTE_WRITE].response = {0},
    [EEPROM_BYTE_WRITE].argSize = 2,  [EEPROM_BYTE_WRITE].dataSize = 1,

        [EEPROM_SERVER_CLIENT].callSize = 2, [EEPROM_SERVER_CLIENT].call = {0x41,0x01},
        [EEPROM_SERVER_CLIENT].respSize = 0, [EEPROM_SERVER_CLIENT].response = {0},
        [EEPROM_SERVER_CLIENT].argSize = 1,  [EEPROM_SERVER_CLIENT].dataSize = 0,

            [EEPROM_SET_SERVER].callSize = 3, [EEPROM_SET_SERVER].call = {0x41,0x01,0x01},
            [EEPROM_SET_SERVER].respSize = 0, [EEPROM_SET_SERVER].response = {0},
            [EEPROM_SET_SERVER].argSize = 0,  [EEPROM_SET_SERVER].dataSize = 0,

            [EEPROM_SET_CLIENT].callSize = 3, [EEPROM_SET_CLIENT].call = {0x41,0x01,0x02},
            [EEPROM_SET_CLIENT].respSize = 0, [EEPROM_SET_CLIENT].response = {0},
            [EEPROM_SET_CLIENT].argSize = 0,  [EEPROM_SET_CLIENT].dataSize = 0,
	    """



