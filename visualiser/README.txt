Here are the new visualiser(s) based on ZMQ, Google
protobufs and pyqtgraph (www.pyqtgraph.org) packages.

Each OROCOS components we want to visualise has it's
corresponding telemetry component. Those telemetry
components take the typekit data and serialize it to a
protobuf message which is sent over ZMQ. ZMQ transport
works as before, PUB-SUB way. We do not use ZMQ topics
(yet), but each component send's data to a different
port (we might want to change this in future).

At the moment, the plotter works with the ops script

/usecases/milan/dmhe_testing.ops

and telemetry port settings are matched with those in
visualiser/vis_sensors_actuators.py.

TODO Port setting should be in some .xml file(s) that
	 should be used both by OROCOS components and the
	 plotter(s).

NOTE: Not all components have to work at once so that
the visualiser can work. Each data source is retrieved
from a separate thread.

Installation:

* Ubuntu packages:
  - Python 2.7, Numpy, Scipy (quite standard)
  - python-qt4 python-qt4-gl python-opengl
  - libzmq1
  - libprotobuf7 protobuf-compiler python-protobufs
  _ !!! At the moment pyqtgraph is compiled from source:
  		git clone git@github.com:pyqtgraph/pyqtgraph.git
		cd pyqtgraph
		sudo python setup.py install 

* OSX: You can use macports and search for corresponding
  	   Ubuntu packages

IMPORTANT: before you use the the app, you will need to
		   generate the python bindings for the protobufs.
		   This, ATM, can be achieved by running the script

		   generate_protobufs.sh

		   TODO: Automate this with tup...
