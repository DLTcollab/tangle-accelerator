#!/usr/bin/env bash

set -uo pipefail

COMMON_FILE="tests/endpoint/common.sh"

if [ ! -f "$COMMON_FILE" ]; then
	echo "$COMMON_FILE is not exists."
	echo "Always execute this script in top-level directory."
	exit 1
fi
source $COMMON_FILE

if [ ! -d "legato" ]; then
	if ! download_legato_repo; then
		echo "Download Legato AF failed. Try to download again..."
		rm -rf legato/ .repo/
		if ! download_legato_repo; then
			echo "Failed to download the Legato AF. Please check your connection"
			exit 1
		fi
	fi
	build_legato_repo
fi
