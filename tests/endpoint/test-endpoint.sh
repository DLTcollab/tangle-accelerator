#!/usr/bin/env bash
# Usage: ./test-endpoint.sh [HOST or IP] [PORT]

if [ "$#" -ne 2 ]; then
	echo "Usage: ./test-endpoint.sh [HOST or IP] [PORT]" >&2
	exit 1
fi

set -euo pipefail

COMMON_FILE="tests/endpoint/common.sh"

if [ ! -f "$COMMON_FILE" ]; then
	echo "$COMMON_FILE is not exists."
	exit 1
fi
source $COMMON_FILE

# Check ip is validity or not
validate_host "$1"
validate_port "$2"

# Create endpoint app
make EP_TARGET=simulator TESTS=true EP_TA_HOST="$1" EP_TA_PORT="$2" legato

# Run endpoint app test here
endpoint/_build_endpoint/localhost/app/endpoint/staging/read-only/bin/endpoint
