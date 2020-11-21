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

TA_HOST="$1"
TA_PORT="$2"
# Check ip is validity or not
validate_host "$1"
validate_port "$2"

# Create endpoint app
make EP_TA_HOST="$TA_HOST" EP_TA_PORT="$TA_PORT" legato

# Start TA server for testing
bazel run //accelerator -- --ta_port="$TA_PORT" --cache=on &
TA=$!
trap "kill -9 ${TA};" INT # Trap SIGINT from Ctrl-C to stop TA

sleep 10

endpoint/_build_endpoint/localhost/app/endpoint/staging/read-only/bin/endpoint --host="$TA_HOST" --port="$TA_PORT"
ret_code=$?

kill -9 ${TA}
exit $ret_code
