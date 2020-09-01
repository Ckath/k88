#!/bin/bash

# bot isnt running yet, reset watchdog file
touch /tmp/k88_alive

# reset bot after it hasnt touched watchdog file for 3 minutes
while true; do
	[ $(($(date +%s)-$(stat -c %Y /tmp/k88_alive))) -gt 180 ] && pkill k88
	sleep 1
done
