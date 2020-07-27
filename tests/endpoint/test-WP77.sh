#!/usr/bin/env bash
# Usage: ./test-WP77.sh [HOST or IP] [PORT]

if [ "$#" -ne 2 ]; then
	echo "Usage: ./test-WP77.sh [HOST or IP] [PORT]" >&2
	exit 1
fi

set -uo pipefail

COMMON_FILE="tests/endpoint/common.sh"

if [ ! -f "$COMMON_FILE" ]; then
	echo "$COMMON_FILE is not exists."
	exit 1
fi
source $COMMON_FILE

# Check ip is validity or not
validate_host "$1"
validate_port "$2"

# setup WP77 leaf shell
setup_leaf "swi-wp77_3.4.0"

make TESTS=true EP_TARGET=wp77xx EP_TA_HOST="$1" EP_TA_PORT="$2" legato &&
	tar zcf endpoint.tgz endpoint/_build_endpoint/wp77xx/app/endpoint/staging/read-only/
