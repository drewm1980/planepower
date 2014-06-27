     Generates synchronized clock signals for all of the other components.

	 This component is intended to be triggered by orocos at a base frequency at
	 the rate of the fastest component of the system.
	 
	 Target frequencies for the output clock signals are read from a properties file.

	 The periods of the output clocks will be rounded UP to the nearest multiple of the
	 base clock frequency.

  
