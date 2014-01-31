
# Colors 
colors = ["#FF0000", "#ADFF2F", "#00BFFF", "#FFFF00", "#FAA460"]

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
def addPlotsToLayout(layout, title, names):
	layout.setContentsMargins(10, 10, 10, 10)
	layout.addLabel( title )
	layout.nextRow()
	
	d = dict()
	for k, v in enumerate( names ):
		plt = layout.addPlot()
		if k < len( names ) - 1:
			plt.hideAxis( "bottom" )
			layout.nextRow()
		
		d[ v ] = plt.plot()
		d[ v ].setPen( colors[ k ] )

	return d
