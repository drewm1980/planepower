Loading new controller gains in a realtime safe manner
is non trivial!

We solve this problem by doing the loading in a separate component
from our higher priority components (this LqrGainLoader component).

Here's how it works:
1. You tell the lqrGainLoader to load in the gains from a properties file (the standard orocos marshalling stuff), from some non-hard deployer code.
	This is NOT hard realtime! ...but will not disturbe the realtime code.
2. You trigger the lqrGainLoader to update once from your non-hard deployer code.
	This is NOT hard realtime! ...but will not disturbe the realtime code.
3. The lqrGainloader sends the gains over a port to the controller.
4. The controller reads from the port when data is available.
		This ~is hard realtime!


