#!/usr/bin/env bash
# Usage: ./test-target.sh [HOST or IP] [PORT] [LEGATO_TARGET]
SUPPORT_TARGETS=('wp77xx')

if [ "$#" -ne 3 ]; then
       echo "Usage: ./test-target.sh [HOST or IP] [PORT] [LEGATO_TARGET]" >&2
       echo "For example: ./test-target.sh localhost 8000 wp77xx" >&2
       exit 1
fi

COMMON_FILE="tests/endpoint/common.sh"

if [ ! -f "$COMMON_FILE" ]; then
       echo "The $COMMON_FILE doesn't exist."
       exit 1
fi
source $COMMON_FILE

# Check ip is validity or not
validate_host "$1"
validate_port "$2"

# Check the target
if ! grep -q "$SUPPORT_TARGETS" <<< "$3"; then
       echo "The target "$3" doesn't support."
       exit 1
fi

set -euo pipefail

if ! env | grep LEAF >/dev/null; then
       echo "error: You must to setup the correct leaf profile and enter the leaf shell first"
       exit 1
fi

make TESTS=true EP_TA_HOST="$1" EP_TA_PORT="$2" EP_TARGET="$3" legato
