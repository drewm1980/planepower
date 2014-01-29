
from collections import deque
from PyQt4.QtCore import QThread, QProcess

from zmq.eventloop import ioloop, zmqstream
import zmq

import Queue

class ZmqSubProtobufWorker( QThread ):
	def __init__(self, address, protobuf, names, queue, bufferSize = 20, topic = ""):
		QThread.__init__( self )

		# TODO assertions and error checking
		self._host_port = address
		self._topic = topic
		self._msg = protobuf()
		self._q = queue

		self._names = names
		
		self._bufferSize = bufferSize
		self._buffer = dict()
		for name in self._names:
			self._buffer[ name ] = deque(maxlen = self._bufferSize)
			for i in range( self._bufferSize ):
				self._buffer[ name ].append( 0 ) 

		# Create ZMQ context
		self._context = zmq.Context().instance()
		# Create socket
		self._socket = self._context.socket( zmq.SUB )
		# Connect to a port
		self._socket.connect( 'tcp://%s' % self._host_port )
		# Configure topic
		self._socket.setsockopt(zmq.SUBSCRIBE, self._topic)

		# Set stopped to false
		self._stopped = False

	def __del__( self ):
		self.wait()

	def run( self ):
		while not self._stopped:
			# TODO This polling method sucks, drains CPU a lot.
			#      Something better should be implemented later.
			if self._socket.poll(timeout = 100) == zmq.POLLIN:
				raw = self._socket.recv( zmq.NOBLOCK )
				self._msg.ParseFromString( raw )

				self.deserialize()

				try:
					self._q.put_nowait( self._buffer )
				except Queue.Full:
					pass

	def deserialize( self ):
		def updateBuffer( name ):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( getattr(self._msg, name) )

		map(updateBuffer, self._names)

	def stop( self ):
		self._stopped = True

# The following code has been adopted from:
# http://stefan.sofa-rockers.org/2012/02/01/designing-and-testing-pyzmq-applications-part-1/

class ZmqWorker( QThread ):
	def __init__( self ):
		QThread.__init__( self )

		self._context = None
		
		self._loop = None

	def setup( self ):
		
		self._context = zmq.Context().instance()
		self._loop = ioloop.IOLoop.instance()

	def stream(self, sockType, address, callback = None, topic = ""):
		
		sock = self._context.socket( sockType )

		# address may be 'host:port' or ('host', port)
		if isinstance(address, str):
			address = address.split(':')
		host, port = address if len( address ) == 2 else (addr[0], None)
		
		sock.connect('tcp://%s:%s' % (host, port))
		
		# Add a default subscription for SUB sockets
		if sockType == zmq.SUB:
			sock.setsockopt(zmq.SUBSCRIBE, topic)
		
		# Create the stream and add the callback
		stream = zmqstream.ZMQStream(sock, self._loop)
		if callback:
			stream.on_recv(callback, copy = True)
		
		return stream, int( port )

class ProtobufWorker( ZmqWorker ):
	def __init__(self, address, protobuf, names, queue, bufferSize = 20):
		ZmqWorker.__init__( self )

		self._address = address
		self._msg = protobuf()
		self._names = names
		self._q = queue

		self._bufferSize = bufferSize
		self._buffer = dict()
		for name in self._names:
			self._buffer[ name ] = deque(maxlen = self._bufferSize)
			for i in range( self._bufferSize ):
				self._buffer[ name ].append( 0 ) 

	def setup( self ):
		super(self.__class__, self).setup()
		
		self._stream, _ = self.stream(zmq.SUB, self._address, self.processData)

	def run( self ):
		self.setup()
		
		self._loop.start()

	def stop( self ):
		self._loop.stop()

	# TODO see copy of the on_recv stuff
	def processData(self, raw):
		
		self._msg.ParseFromString( raw[ 0 ] )
			
		def updateBuffer( name ):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( getattr(self._msg, name) )

		map(updateBuffer, self._names)

		try:
			self._q.put_nowait( self._buffer )
		except Queue.Full:
			pass

