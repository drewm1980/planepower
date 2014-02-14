
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
			t = t + coloredText(v, colors[ n ])
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
		
		d[ name ] = plt.plot()
		d[ name ].setPen( colors[ k ] )

		if name in options:
			if "semilogy" in options[ name ]:
				plt.setLogMode(y = True)

	return d
