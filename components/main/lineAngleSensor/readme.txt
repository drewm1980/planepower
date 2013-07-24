This is the orocos component for communicating with the indoor line angle sensor.

This component reads the analog inputs of the soem ebox, does some unit
conversion, and outputs the values on ports.

At the time of writing, the circuitry is based on a 12VDC wall wart, driving
the potiometers acting as voltage dividers.  A pair of AD622 instrumentation
op-amps buffer the signal that is sent to the analog inputs of the ebox.
They are currently configured for unity gain, but this is a matter of
calibration and may change.
