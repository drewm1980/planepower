Notes on realtime:

The lowest hanging fruit is to set your cpu governor to "performance" to pin the clock speed to it's maximum, and to disable hyperthreading (in the BIOS, on intel).

Furthermore, on a realtime (PREEMPT_RT) kernel...

High priority tasks should be using the FIFO scheduler
FIFO tasks are scheduled before all non-FIFO tasks.
There is a separate "rtprio" realtime priority for FIFO tasks.

You can set both FIFO and the realtime priority of a running task with:

chrt -f -p <prio> <pid>

Our helper functions just shell out to call this function.

To list FIFO threads:
ps -eLo cls,rtprio,pid,cmd | grep "^ FF" | sort -r

To list interrupts to get more info about what irqs are associated with which drivers:
cat /proc/interrupts

Without setting up anything, 50 is the default rtprio for irq's

Unless we have a good reason, we should probably set the same irq handler
priorities that OSADL uses!
