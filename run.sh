#!/bin/sh
while true; do
	./k88
	sleep 1
	echo "$(date) restarted" > run.log
done
