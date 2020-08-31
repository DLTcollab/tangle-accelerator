#!/usr/bin/env bash

SUDO=false
if which sudo &>/dev/null; then
	SUDO=true
fi

# create vcan
if [ "$SUDO" == true ]; then
	sudo ip link add dev vcan0 type vcan
	sudo ifconfig vcan0 up
else
	ip link add dev vcan0 type vcan
	ifconfig vcan0 up
fi
