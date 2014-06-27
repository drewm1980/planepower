The LEDTracker OROCOS component captures images from a pair of firewire
cameras, and finds the location of three LED markers in the images.  The
emphasis is on speed, rather than robustness.

There are a couple test programs that ONLY talk to the cameras, and can be
safely used in conjunction with, e.g. voltageController's test script.

For some reason sometimes the cameras forget there settings.  The settings
are slightly dependent on outside illumination but here is a good starting point.

(In coriander "Controls" tab)
Brightness: Man 0
Auto exposure: OFF
Sharpness: OFF
Gamma: Abs 1.0
Shutter: Man XXX		IMPORTANT; this is the main thing you should tune.
Gaint: Man 64  
