#!/bin/sh
while true; do
	echo "$(date) session started" > run.log
	./k88
	sleep 1
	echo "$(date) restarted" >> run.log
done
