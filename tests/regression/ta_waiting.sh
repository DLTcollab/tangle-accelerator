#!/usr/bin/env bash

socket=$1

# Whether tangle-accelerator successfully initialzed
success=0

start_notification="TA-START"
# Wait until tangle-accelerator has been initialized
echo "==============Wait for TA starting=============="
while read -r line; do
	if [[ "$line" == "$start_notification" ]]; then
		echo "nc info: $line"
		success=1
	fi
done <<<$(nc -U -l $socket | tr '\0' '\n')
echo "==============TA has successfully started=============="

if [ "$success" -eq 1 ]; then
	exit 0
else
	exit 1
fi
