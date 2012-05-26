#!/bin/bash
# watchdog script to prevent mctracer from going nuts

pidregex='([0-9]+).*\) Z ([0-9]+)[^)]*$'
parregex='Name:\s+mctracer.*\sState:\s+R \(running\).*\sUid:\s+'$UID

function checkprocs {
	for proc in $(ps axo pid); do if [[ -r /proc/$proc/status ]]; then # perhaps change to ps ao pid for optimization
		status=$(cat /proc/$proc/stat)
		if [[ $status =~ $pidregex ]]; then
			#nice, we've caught a defunct process and it's owner
			pid=${BASH_REMATCH[1]}
			ppid=${BASH_REMATCH[2]}
			parstatus=$(cat /proc/$ppid/status) # check the status and not stat because of the UID
			if [[ $parstatus =~ $parregex ]]; then
				# the parent does also seem to be mctracer, kill it with <strike>fire</strike> a nice and stealthy sleep arrow
				kill -$1 $ppid
				echo $(date) "- $1ing process $ppid with defunct child $pid"
			fi
		fi
	fi; done
}


trap "parregex='Name:\s+mctracer.*\sUid:\s+'$UID ; checkprocs KILL ; exit" EXIT SIGINT SIGSTOP SIGHUP SIGTERM

while true; do
	checkprocs STOP
	sleep 7
done
