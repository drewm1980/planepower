
Note on running stuff with realtime permissions:
To run with realtime priorities, you need to have root permissions.
HOWEVER, you also want to still have all of your user-specific environment variables
(which where mostly set by your ~current version of planepower/env.sh).
To achieve this, do something like the following:

sudo /bin/bash
./main.lua

