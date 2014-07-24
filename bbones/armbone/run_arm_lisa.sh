#!/bin/sh
### BEGIN INIT INFO
# Provides:          main_arm_lisa
# Short-Description: Intscript for the HIGHWIND arm
# Description:       runs the main_arm_lisa binary on start, kills ALL of them on stop.
#                    
### END INIT INFO


case "$1" in
    start)
	/root/planepower/bbones/armbone/main_arm_lisa 192.168.1.4 7777 &disown
    ;;

    stop)
	killall -9 main_arm_lisa
    ;;
esac

exit 0
# Make sure to do something like: 
# update-rc.d  run_arm_lisa.sh defaults
# update-rc.d  run_arm_lisa.sh enable
