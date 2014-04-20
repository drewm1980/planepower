#!/usr/bin/env python

import sys
from PyQt4 import QtGui

class FileFolderDialog( QtGui.QMainWindow ):
	"""
	Adapted from zetcode.com
	"""

	def __init__(self):
		super(FileFolderDialog, self).__init__()

	def getFile(self, startFolder = '.'):
		self._files = QtGui.QFileDialog.getOpenFileName(self, 'Open file', '.')
		return self._files

	def getFolder(self, startFolder = '.'):
		self._folders = QtGui.QFileDialog.getExistingDirectory(self, 
                                                         	'Open folder', 
														 	'.', 
														 	QtGui.QFileDialog.ShowDirsOnly)
		
		return self._folders
	
class YesNoDialog( QtGui.QMainWindow ):
	"""
	A simple yes/no dialog
	"""
	
	def __init__(self, message):
		super(YesNoDialog, self).__init__()
		self._message = message
		
		reply = QtGui.QMessageBox.question(self, 'Message', 
					self._message, QtGui.QMessageBox.Yes, QtGui.QMessageBox.No)
		
		if reply == QtGui.QMessageBox.Yes:
			self._reply = True
		else:
			self._reply = False
		
	def getReply(self):
		return self._reply                     

if __name__ == '__main__':
	"""
	Just for testing
	"""
	
	app = QtGui.QApplication(sys.argv)
	ex = FileFolderDialog()
	
	print ex.getFile()
	print ex.getFolder()

	msg = YesNoDialog("Do you want a beer?")
	print msg.getReply()