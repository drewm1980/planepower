#!/usr/bin/env python

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
		error("Response of wrong size!")
	response = response_and_data[0:respSize]
	data = response_and_data[respSize:]
	if response != expectedResponse:
		error("Response did not match expected response!")
	return data

# Check if a device is responsive by going into
# and back out of commmand mode
def test_device_alive(device):
	send_at_command_raw(device,ENTER_COMMAND_MODE)
	send_at_command_raw(device,EXIT_COMMAND_MODE)

# Send an AT command.
# Handles entering and exiting command mode for you.
def send_at_command(device,command,arguments):
	send_at_command_raw(device,ENTER_COMMAND_MODE)
	data = send_at_command_raw(device,command,arguments)
	send_at_command_raw(device,EXIT_COMMAND_MODE)
	return data

def read_temperature(device):
	data = send_at_command(READ_TEMPERATURE)
	return data

	
