#!/usr/bin/env bash

set -euo pipefail

SUDO=false
if which sudo &>/dev/null; then
	SUDO=true
fi

# Bring up virtual CAN interface  
if [ "$SUDO" == true ]; then
	sudo ip link add dev vcan0 type vcan
	sudo ifconfig vcan0 up
else
	ip link add dev vcan0 type vcan
	ifconfig vcan0 up
fi

if [[ $(ip link | grep vcan0) = 0 ]]; then
  echo "The virtual CAN interface is not activated"
  echo "Please check your kernel configuration"
  exit 1
fi
