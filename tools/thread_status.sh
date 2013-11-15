#!/usr/bin/env bash
echo "This program displays info about how orocos is currently using the cpus and memory."
orocos_pid=`pgrep deployer-gnulin`
echo "Info about orocos's threads (from pgrep and pstree):"
pstree $orocos_pid -p

run_command(){
	echo ~~~~~~~~$cmd~~~~~~~
	$cmd
}

function get_stats {
	sudo cat /proc/$1/stat | cut -d ' ' -f 1,3,41
}

#cmd="sudo cat /proc/$orocos_pid/numa_maps"
#cmd="sudo cat /proc/$orocos_pid/latency"
cmd="sudo cat /proc/$orocos_pid/sched"
run_command
#cmd="sudo cat /proc/$orocos_pid/schedstat"
cmd="sudo cat /proc/$orocos_pid/status"
run_command
cmd="sudo cat /proc/$orocos_pid/stat | cut -d ' ' -f 1,3,42"
run_command

for tid in `ls -1 /proc/$orocos_pid/task | sort`; do
	get_stats $tid
done

#for tid in `ls -1 /proc/$orocos_pid/task | sort`; do
	#gdb attach $tid -ex bt -ex cont
#done


