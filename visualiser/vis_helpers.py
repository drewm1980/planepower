
import zmq_protobuf_helpers
import numpy as np

from collections import OrderedDict
from textwrap import fill

def flatten( iterables ):
	t = []
	for it in iterables:
		if isinstance(it, str) or isinstance(it, tuple):
			t.extend( [ it ] )
		else:
			assert isinstance(it, list)
			t.extend( it )
	return t

# Colors 
colors = ["#FF0000", "#ADFF2F", "#00BFFF", "#FFFF00", "#FAA460"]
symbols = OrderedDict([("o", "&#x25CF;"),
					   ("s", "&#x25FC;"),
					   ("t", "&#x25BC;"),
					   ("d", "&#x25C6;"),
					   ("+", "+")])


def coloredText(txt, clr):
	t = "<span style='color: " + clr + "; font-weight: bold'>" + str( txt ) + "</span>"
	return t

def redBoldText( txt ):	
	t = "<span style='color: " + colors[ 0 ] + "; font-weight: bold'>" + str( txt ) + "</span>"
	return t

def greenBoldText( txt ):
	t = "<span style='color: " + colors[ 1 ] + "; font-weight: bold'>" + str( txt ) + "</span>"
	return t

def blueBoldText( txt ):
	t = "<span style='color: " + colors[ 2 ] + "; font-weight: bold'>" + str( txt ) + "</span>"
	return t

def wrappedCounter( n ):
	counter = 0
	while True:
		yield counter % n
		counter += 1

def addPlotsToLayout(layout, title, names, options = dict()):
	"""
	A helper function to make plot objects on a grid
	"""
	
	layout.setContentsMargins(10, 10, 10, 10)
	if isinstance(title, str):
		layout.addLabel( title )
		layout.nextRow()
	elif isinstance(title, list):
		t = []
		clr = wrappedCounter( len( colors ) )
		for v in title:
			dim = 0

			def addSingleString(v, c, s = None):
				if isinstance(v, str):
					_v = v
				elif isinstance (v, tuple):
					# First element of the tuple is taken as the name and
					# the second one as unit
					_v = v[ 0 ] + " [" + v[ 1 ] + "]"
				else:
					raise TypeError("Title item can be a string or a tuple")

				if s is not None:
					_v += " " + s[ 1 ]

				t.extend( [coloredText(_v, colors[ c ])] )

				return len( _v )
			
			if isinstance(v, str) or isinstance(v, tuple):
				dim += addSingleString(v, next( clr ))
			elif isinstance(v, list):
				cc = next( clr )
				ss = symbols.iteritems()
				for innerName in v:
					dim += addSingleString(innerName, cc, next( ss ) )
			else:
				raise TypeError("Title items in the outer-most list can be: string, tuple, or a list")

			if dim > 30:
				layout.addLabel( ", ".join( t ) )					
				layout.nextRow()
				t = []
		
		if len( t ):
			layout.addLabel( ", ".join( t ) )					
			layout.nextRow()

	else:
		raise TypeError("Title can be a string or a list of strings")
	
	d = dict()
	clr = wrappedCounter( len( colors ) )
	for k, name in enumerate( names ):
		plt = layout.addPlot()
		if k < len( names ) - 1:
			plt.hideAxis( "bottom" )
			layout.nextRow()

		def addSinglePlot(name, c, s = None):
			if isinstance(name, str):
				_name = name
			elif isinstance(name, tuple):
				_name = name[ 0 ]
			else:
				raise TypeError("Items in the inner-most list can be either strings or tuples")
			_clr = colors[ c ]
			if s is None:
				d[ _name ] = plt.plot(pen = _clr)
			else:
				# Add a bit of transparency to the symbol color
				_clrT  = _clr + "80"
				d[ _name ] = plt.plot(pen = _clr, \
									  symbol = s[ 0 ], symbolPen = _clrT, symbolBrush = _clrT, symbolSize = 7)

			if s is not None:
				d[ _name ].setSymbol( s[ 0 ] )

			if name in options:
				if "semilogy" in options[ _name ]:
					plt.setLogMode(y = True)

		if isinstance(name, str) or isinstance(name, tuple):
			addSinglePlot(name, next( clr ) )
		elif isinstance(name, list):
			cc = next( clr )
			ss = symbols.iteritems()
			for innerName in name:
				addSinglePlot(innerName, cc, next( ss ))
		else:
			raise TypeError("Items in the outer-most list can be: string, tuple, or a list")

	return d

class OcpWorker( zmq_protobuf_helpers.ZmqSubProtobufWorker ):
	"""
	A class for deserialization of the OCP-like protobuf
	+ some more useful stuff
	
	NOTE: This deserialization is tightly coupled to the (auto-)generated
	      protobuf specs in rawesome (see rawe.ocp.Ocp, function generateProto)
	"""
	
	def __init__(self, protoModule, address, queue, bufferSize = 20, topic = ""):
		
		from types import ModuleType
		assert isinstance(protoModule, ModuleType)
		# This is "a convention"
		assert protoModule.__name__.endswith("Telemetry_pb2")
		
		moduleName = protoModule.__name__[: protoModule.__name__.find("Telemetry_pb2")]
		# This is another convention
		msgName = moduleName + "Msg"
		
		xNames = getattr(protoModule, "_" + msgName.upper() + "_XNAMES")
		zNames = getattr(protoModule, "_" + msgName.upper() + "_ZNAMES")
		uNames = getattr(protoModule, "_" + msgName.upper() + "_UNAMES")
		yNames = getattr(protoModule, "_" + msgName.upper() + "_YNAMES")
		
		# Make the xNames enum dictionary, while removing "idx_" from keys 
		self._xMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in xNames.values)
		# ... and do same for zNames, uNames, yNames
		self._zMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in zNames.values)
		self._uMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in uNames.values)
		self._yMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in yNames.values)

		self._simpleFieldNames = ["solver_status", "kkt_value", "obj_value", "n_asc",
								  "exec_fdb", "exec_prep", "ts_trigger", "ts_elapsed"]

		msgClass = getattr(protoModule, msgName)

		zmq_protobuf_helpers.ZmqSubProtobufWorker.__init__(
														self, address, msgClass, self._simpleFieldNames,
									  					queue, bufferSize, topic)
		
		# Define new buffers for horizons of d. states + RPY, a. states, and control
		for k in self._xMap.keys() + ["roll", "pitch", "yaw"]:
			self._buffer[ k ] = list( xrange(self._msg.N + 1) )
		for k in self._zMap.keys() + self._uMap.keys():
			self._buffer[ k ] = list( xrange( self._msg.N ) )

		# Define buffer for direct measurements/references of states and controls
		self._yxMap = dict()
		self._yuMap = dict()
		for k, v in self._yMap.items():
			_name = "y_" + k
			if k in self._xMap.keys():
				self._buffer[ _name ] = list( xrange(self._msg.N ) ) # TODO extend to N+1
				self._yxMap.update({k: (_name, v)})
			elif k in self._uMap.keys():
				self._buffer[ _name ] = list( xrange( self._msg.N ) )
				self._yuMap.update({k: (_name, v)})

	def deserialize( self ):
		def updateBufferSimple( name ):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( getattr(self._msg, name) )
		
		map(updateBufferSimple, self._simpleFieldNames)
		
		def updateHorizonBuffers(nMap, name):
			el = getattr(self._msg, name)
			for k, v in nMap.items():
				self._buffer[ k ] = el[ v ].h._values

		def updateHorizonBuffersYYN():
			for k, v in self._yxMap.items():
				self._buffer[ v[ 0 ] ] = self._msg.y[ v[ 1 ] ].h._values
			for k, v in self._yuMap.items():
				self._buffer[ v[ 0 ] ] = self._msg.y[ v[ 1 ] ].h._values
		
		updateHorizonBuffers(self._xMap, "x")
		updateHorizonBuffers(self._zMap, "z")
		updateHorizonBuffers(self._uMap, "u")

		# TODO include data from yN buffer as well
		updateHorizonBuffersYYN()

		#
		# And now do some unit conversions to make our lives easier
		#
		
		for n in xrange(self._msg.N + 1):
			# yaw = atan2(e12, e11)
			self._buffer["yaw"][ n ] = np.rad2deg( np.arctan2(self._buffer["e12"][ n ], self._buffer["e11"][ n ]) )
			# pitch = real(asin(-e13))
			self._buffer["pitch"][ n ] = np.rad2deg( np.arcsin( -self._buffer["e13"][ n ] ) )
			# roll = atan2(e23, e33)
			self._buffer["roll"][ n ] = np.rad2deg( np.arctan2(self._buffer["e23"][ n ], self._buffer["e33"][ n ]) )

#		for name in ["aileron", "daileron", "elevator", "delevator"]:
#			self._buffer[ name ] = np.rad2deg( self._buffer[ name ] )

		self._buffer[ "ddelta" ] = np.array(self._buffer[ "ddelta" ]) * 60.0 / (2.0 * np.pi)

	def getSimpleFieldNames( self ):
		return self._simpleFieldNames
