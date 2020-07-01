#!/usr/bin/env bash

set -uo pipefail

COMMON_FILE="tests/endpoint/common.sh"

if [ ! -f "$COMMON_FILE" ]
then
    echo "$COMMON_FILE is not exists."
    echo "Always execute this script in top-level directory."
    exit 1
fi
source $COMMON_FILE

download_legato_repo
build_legato_repo
