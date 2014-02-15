
#from exception import TypeError

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

# A helper function to make plot objects on a grid
def addPlotsToLayout(layout, title, names, options = dict()):
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
