#!/usr/bin/env python

from time import sleep

from rm024_atcommands import *

# This is a library for talking to rm024 modules with python.

# Send an AT command while already in command mode
# device is a pyserial device that is already open
def send_at_command_raw(device,command,arguments=''):
	call = command['call']
	callSize = command['callSize']
	expectedResponse = command['response']
	respSize = command['respSize']
	argSize = command['argSize']
	dataSize = command['dataSize']

	msg = call + arguments
	device.write(msg)
	response_and_data = device.read(respSize + dataSize)
	if len(response_and_data) != respSize + dataSize:
		raise Exception("Response of wrong size!")
	response = response_and_data[0:respSize]
	data = response_and_data[respSize:]
	if response != expectedResponse:
		raise Exception("Response did not match expected response!\n"
				+ "Expected Response: " + repr(expectedResponse) + '\n'
				+ "Actual Response: " + repr(response))
	return data

# Check if a device is responsive by going into
# and back out of commmand mode
def test_device_alive(device):
	send_at_command_raw(device,ENTER_COMMAND_MODE)
	send_at_command_raw(device,EXIT_COMMAND_MODE)

# Send an AT command.
# Handles entering and exiting command mode for you.
def send_at_command(device,command,arguments=''):
	device.flushOutput()
	device.flushInput()
	sleep(.015)
	send_at_command_raw(device,ENTER_COMMAND_MODE)
	data = send_at_command_raw(device,command,arguments)
	send_at_command_raw(device,EXIT_COMMAND_MODE)
	return data

def read_temperature(device):
	data = send_at_command(READ_TEMPERATURE)
	return data

statusDict = {'\x02':'Server','\x03':'Client in Range','\x01':'Client out of Range'}

def set_to_server_on_fly(device):
	data = send_at_command(device,SET_TO_SERVER)
	print "firmware version is: " + str(ord(data[0]))
	status = data[1]
	print "Device reports status:" + statusDict[status]

def set_to_client_on_fly(device):
	data = send_at_command(device,SET_TO_CLIENT)
	print "firmware version is: " + str(ord(data[0]))
	status = data[1]
	print "Device reports status:" + statusDict[status]

def get_last_rssi(device):
	rssi = send_at_command(device,GET_LAST_RSSI)
	print "Device reports signal strength " + str(ord(rssi)) + '\tof ' + str(ord('\xFF'))

# Merge a command with a subcommand.
# Used mostly for writing to EEPROM and IRAM addresses.
# WARNING! SOME SUBCOMMANDS HAVE A BITMASK APPENDED TO THEIR command that require special casing.
def merge_commands(command,subcommand):
	c = {}
	c['call'] = command['call'] + subcommand['call']
	c['callSize'] = command['callSize'] + subcommand['callSize']
	c['respSize'] = command['respSize'] + subcommand['respSize']
	c['response'] = command['response'] + subcommand['response']
	c['argSize'] = subcommand['argSize']
	c['dataSize'] = command['dataSize'] + subcommand['dataSize']
	return c

# Set the rf packet size, in bytes, python number, 6-239 for 500kbps rate
# Note: Requires Autoconfiguration to be turned off, and that can only be done in EEPROM
def set_rf_packet_size_on_fly(device, size):
	if size < 6 or size > 239:
		raise Exception("RF packet size must be between 6 and 255")
	command = merge_commands(WRITE_IRAM,IRAM_RF_PACKET_SIZE)
	data = send_at_command(device,command,chr(size))
	
# Similar to set_rf_packet_size_on_fly, but stores the value in the EEPROM.
# Requires reset.
def set_rf_packet_size_eeprom(device, size):
	if size < 6 or size > 239:
		raise Exception("RF packet size must be between 6 and 255")
	command = merge_commands(EEPROM_BYTE_WRITE,EEPROM_RF_PACKET_SIZE)
	data = send_at_command(device,command,chr(size))

# Determine if autoconfiguration is currently enabled
def get_autoconfig(device):
	command = merge_commands(EEPROM_BYTE_READ,EEPROM_AUTO_CONFIG)
	command['callSize'] -= 1
	bitmask = command['call'][-1:]
	command['call'] = command['call'][:-1]
	start,length,currentbyte = send_at_command(device,command)
	return bool(ord(currentbyte) & ord(bitmask))

# Turn off autoconfiguration of rf packet size.
# enable is a python boolean.
def set_autoconfig(device,enable):
	command = merge_commands(EEPROM_BYTE_READ,EEPROM_AUTO_CONFIG)
	command['callSize'] -= 1
	bitmask = command['call'][-1:]
	command['call'] = command['call'][:-1]
	start,length,currentbyte = send_at_command(device,command)

	command = merge_commands(EEPROM_BYTE_WRITE,EEPROM_AUTO_CONFIG)
	command['callSize'] -= 1
	bitmask = command['call'][-1:]
	command['call'] = command['call'][:-1]
	if enable:
		newbyte = chr(ord(currentbyte) | ord(bitmask))
	else:
		newbyte = chr(ord(currentbyte) & ~ ord(bitmask))
	data = send_at_command(device,command,newbyte)
	

