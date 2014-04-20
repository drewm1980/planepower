#!/usr/bin/env python

import sys, os
from PyQt4 import QtGui

import subprocess, shlex

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
	
def savePlotsToPdf(figures, folder, prefix = None, ext = ".png", dpi = 150):
	"""
	A simple function to generate a PDF file out of matplotlib
	generated figures.
	"""
	
	assert( os.path.isdir( folder ) )
	
	#
	# Get the log name
	#
	logName = folder.split("/")[ -1 ]
	
	#
	# Export all figures 
	#
	plots = ""
	for it, fig in enumerate(figures):
		name = logName + "_" + repr( it )
		
		fig.savefig(name + ext, dpi = dpi)
		
		plots += "\includegraphics[width=\\textwidth]{" + name + "}\n"
		
	tex = r'''
\documentclass[12pt,a4paper]{article}
\usepackage[utf8]{inputenc}
\usepackage{graphicx}
\usepackage[left=2cm,right=2cm,top=2cm,bottom=2cm]{geometry}
\begin{document}
%(plots)s
\end{document}
''' % {"plots": plots}

	if prefix == None:
		texFileName = ""
	else:
		assert isinstance(prefix, str)
		texFileName = prefix + "_"
		
	texFileName += logName + ".tex" 
	with open(texFileName, 'w') as f:
		f.write( tex )
	proc = subprocess.Popen(shlex.split("pdflatex " + texFileName))
	proc.communicate()
	
	# Delete all figures
	for it, fig in enumerate(figures):
		name = logName + "_" + repr( it )
		os.remove(name + ext)					 

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