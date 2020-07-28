#!/bin/sh
echo "$(date) session started" > run.log
while true; do
	./k88
	sleep 1
	echo "$(date) restarted" >> run.log
done
