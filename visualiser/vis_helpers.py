
import zmq_protobuf_helpers
import numpy as np

# Colors 
colors = ["#FF0000", "#ADFF2F", "#00BFFF", "#FFFF00", "#FAA460"]

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

def addPlotsToLayout(layout, title, names, options = dict()):
	"""
	A helper function to make plot objects on a grid
	"""
	
	layout.setContentsMargins(10, 10, 10, 10)
	if isinstance(title, str):
		layout.addLabel( title )
	elif isinstance(title, list):
		t = ""
		for n, v in enumerate( title ):
			if isinstance(v, str):
				_v = v
			elif isinstance (v, tuple):
				# First element of the tuple is taken as the name and
				# the second one as unit
				_v = v[ 0 ] + " [" + v[ 1 ] + "]"
			else:
				raise TypeError("Title item can be a string or a tuple")
			
			t = t + coloredText(_v, colors[ n ])
			if n < len( title ) - 1:
				t = t + ", "
		
		layout.addLabel( t )
	else:
		raise TypeError("Title can be a string or a list of strings")
	
	layout.nextRow()
	
	d = dict()
	for k, name in enumerate( names ):
		plt = layout.addPlot()
		if k < len( names ) - 1:
			plt.hideAxis( "bottom" )
			layout.nextRow()

		if isinstance(name, str):
			_name = name
		elif isinstance(name, tuple):
			_name = name[ 0 ]
		else:
			raise TypeError("Items in the list of names can be either strings or tuples")
		
		d[ _name ] = plt.plot()
		d[ _name ].setPen( colors[ k ] )

		if name in options:
			if "semilogy" in options[ _name ]:
				plt.setLogMode(y = True)

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

		self._simpleFieldNames = ["solver_status", "kkt_value", "obj_value", "n_asc"
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

	def deserialize( self ):
		def updateBufferSimple( name ):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( getattr(self._msg, name) )
		
		map(updateBufferSimple, self._simpleFieldNames)
		
		def updateHorizonBuffers(nMap, name):
			el = getattr(self._msg, name)
			for k, v in nMap.items():
				self._buffer[ k ] = el[ v ].h._values
		
		updateHorizonBuffers(self._xMap, "x")
		updateHorizonBuffers(self._zMap, "z")
		updateHorizonBuffers(self._uMap, "u")

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

		for name in ["aileron", "daileron", "elevator", "delevator"]:
			self._buffer[ name ] = np.rad2deg( self._buffer[ name ] )

		self._buffer[ "ddelta" ] = np.array(self._buffer[ "ddelta" ]) * 60.0 / (2.0 * np.pi)

	def getSimpleFieldNames( self ):
		return self._simpleFieldNames