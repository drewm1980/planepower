#!/bin/sh
### BEGIN INIT INFO
# Provides:          main_arm
# Short-Description: Intscript for the HIGHWIND arm
# Description:       runs the main_arm binary on start, kills ALL of them on stop.
#                    
### END INIT INFO



case "$1" in
    start)
	/root/planepower/bbones/armbone/main_arm 192.168.1.4 8888 &disown
    ;;

    stop)
	killall -9 main_arm
    ;;
esac

exit 0
