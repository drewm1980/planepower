#!/usr/bin/env python

import sys, os, shutil

import netCDF4 as nc
import numpy as np
import scipy.io as sio

def cli_progress(info, val, end_val, bar_length=20):
	"""
	A simple command line progress bar

	http://stackoverflow.com/questions/3173320/text-progress-bar-in-the-console
	"""
	percent = float( val ) / end_val
	hashes = '#' * int(round(percent * bar_length))
	spaces = ' ' * (bar_length - len(hashes))
	sys.stdout.write("\r{0}: [{1}] {2}%".format(info, hashes + spaces, int(round(percent * 100))))
	sys.stdout.flush()

def getTimeStampKey( keys ):
	for k in keys:
		if k.split( '.' )[ -1 ] == "ts_trigger":
			return k
	raise TypeError("There is no key named ts_trigger.")

def processNetCdfFile( fileName ):
	assert os.path.isfile( fileName )
	name = os.path.basename( fileName )
	info = "Processing file " + name

	# Open file in read mode
	h = nc.Dataset(f, 'r')
	keys = h.variables.keys()
	numKeys = len( keys )
	# There are duplicates in the log file and here we want to get rid of them
	_, indices = np.unique(h.variables[ getTimeStampKey( keys ) ], return_index=True)

	res = {}
	for it, k in enumerate( keys ):
		_k = k.split( '.' )

		var = h.variables[ k ][ indices ]
		if len( _k ) == 1:
			# Simple names
			res[ k ] = var
		elif _k[ -1 ].isdigit() is True:
			if int(_k[ -1 ]) is 0:
				# The new array is in the play
				res[ _k[ -2 ] ] = np.reshape(var, (len( indices ), 1))
			else:
				# We are concatenating a column
				res[ _k[ -2 ] ] = np.append(res[ _k[ -2 ] ], var.reshape(len( indices ), 1), axis = 1)
		else:
			# The new scalar values is in the play
			res[ _k[ -1 ] ] = var

		# Update the progress bar
		cli_progress(info, it + 1, numKeys)
	
	return res

def saveDataToMatFile(data, fileName):
	"""
	Save the dictionary to the .mat file
	"""
	sio.savemat(fileName, data, oned_as = "column")

if __name__ == '__main__':
	fileNames = sys.argv[1:]
	for f in fileNames:
		# Process the file
		data = processNetCdfFile( f )
		# Save to a .mat file
		name = os.path.basename( f )
		folder = os.path.dirname( f )
		matName = os.path.join(folder, os.path.splitext( name )[ 0 ] + ".mat")
		saveDataToMatFile(data, matName)
